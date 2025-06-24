/**
 * TupleDNS v1.0 - Core Implementation
 * 
 * Portable C99 implementation of the TupleDNS protocol.
 */

#define _POSIX_C_SOURCE 200112L  /* For getaddrinfo */
#define _GNU_SOURCE              /* For strdup on some systems */

#include "tupledns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

/* Provide strdup if not available */
#ifndef _GNU_SOURCE
static char* tuple_strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* dup = malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}
#define strdup tuple_strdup
#endif

/* Global Configuration */
static tupledns_config_t g_config = {0};
static int g_initialized = 0;
static tupledns_error_t g_last_error = TUPLEDNS_OK;

/* Internal Structures */
typedef struct dns_query_ctx {
    char* hostname;
    char* ip_address;
    char** txt_records;
    int txt_count;
    int completed;
    tupledns_error_t error;
} dns_query_ctx_t;

/* ========================================================================
 * INITIALIZATION AND CONFIGURATION
 * ======================================================================== */

tupledns_config_t tupledns_default_config(void) {
    tupledns_config_t config = {
        .timeout = TUPLEDNS_DEFAULT_TIMEOUT,
        .max_concurrent = 16,
        .enable_caching = 1,
        .cache_ttl = TUPLEDNS_DEFAULT_TTL
    };
    return config;
}

int tupledns_init(const tupledns_config_t* config) {
    if (g_initialized) {
        return TUPLEDNS_OK;
    }
    
    if (config) {
        g_config = *config;
    } else {
        g_config = tupledns_default_config();
    }
    
    g_initialized = 1;
    g_last_error = TUPLEDNS_OK;
    return TUPLEDNS_OK;
}

void tupledns_cleanup(void) {
    g_initialized = 0;
    memset(&g_config, 0, sizeof(g_config));
}

int tupledns_set_config(const tupledns_config_t* config) {
    if (!config) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return TUPLEDNS_ERROR_INVALID_PARAMETER;
    }
    g_config = *config;
    return TUPLEDNS_OK;
}

tupledns_config_t tupledns_get_config(void) {
    return g_config;
}

/* ========================================================================
 * ERROR HANDLING
 * ======================================================================== */

const char* tupledns_error_string(tupledns_error_t error) {
    switch (error) {
        case TUPLEDNS_OK:
            return "Success";
        case TUPLEDNS_ERROR_INVALID_COORDINATE:
            return "Invalid coordinate format";
        case TUPLEDNS_ERROR_DNS_QUERY_FAILED:
            return "DNS query failed";
        case TUPLEDNS_ERROR_MEMORY_ALLOCATION:
            return "Memory allocation failed";
        case TUPLEDNS_ERROR_INVALID_PARAMETER:
            return "Invalid parameter";
        case TUPLEDNS_ERROR_TIMEOUT:
            return "Query timeout";
        case TUPLEDNS_ERROR_NO_RESULTS:
            return "No results found";
        case TUPLEDNS_ERROR_CAPABILITY_PARSE:
            return "Capability parsing error";
        default:
            return "Unknown error";
    }
}

tupledns_error_t tupledns_get_last_error(void) {
    return g_last_error;
}

/* ========================================================================
 * STRING UTILITIES
 * ======================================================================== */

char** tupledns_split_string(const char* str, const char* separator, int* count) {
    if (!str || !separator || !count) {
        return NULL;
    }
    
    *count = 0;
    char* str_copy = strdup(str);
    if (!str_copy) {
        return NULL;
    }
    
    /* Count tokens */
    char* token = strtok(str_copy, separator);
    while (token) {
        (*count)++;
        token = strtok(NULL, separator);
    }
    
    if (*count == 0) {
        free(str_copy);
        return NULL;
    }
    
    /* Allocate array */
    char** result = calloc(*count, sizeof(char*));
    if (!result) {
        free(str_copy);
        *count = 0;
        return NULL;
    }
    
    /* Copy string again and split */
    strcpy(str_copy, str);
    token = strtok(str_copy, separator);
    int i = 0;
    while (token && i < *count) {
        result[i] = strdup(token);
        if (!result[i]) {
            tupledns_free_string_array(result, i);
            free(str_copy);
            *count = 0;
            return NULL;
        }
        token = strtok(NULL, separator);
        i++;
    }
    
    free(str_copy);
    return result;
}

char* tupledns_join_strings(const char* strings[], int count, const char* separator) {
    if (!strings || count <= 0 || !separator) {
        return NULL;
    }
    
    size_t total_len = 0;
    size_t sep_len = strlen(separator);
    
    /* Calculate total length */
    for (int i = 0; i < count; i++) {
        if (strings[i]) {
            total_len += strlen(strings[i]);
            if (i < count - 1) {
                total_len += sep_len;
            }
        }
    }
    
    char* result = malloc(total_len + 1);
    if (!result) {
        return NULL;
    }
    
    result[0] = '\0';
    for (int i = 0; i < count; i++) {
        if (strings[i]) {
            strcat(result, strings[i]);
            if (i < count - 1) {
                strcat(result, separator);
            }
        }
    }
    
    return result;
}

void tupledns_free_string_array(char** strings, int count) {
    if (!strings) return;
    
    for (int i = 0; i < count; i++) {
        free(strings[i]);
    }
    free(strings);
}

/* ========================================================================
 * COORDINATE HANDLING
 * ======================================================================== */

int tupledns_validate_coordinate(const char* coordinate) {
    if (!coordinate) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return 0;
    }
    
    size_t len = strlen(coordinate);
    if (len == 0 || len > TUPLEDNS_MAX_COORDINATE_LENGTH) {
        g_last_error = TUPLEDNS_ERROR_INVALID_COORDINATE;
        return 0;
    }
    
    /* Check if ends with .tuple */
    const char* suffix = ".tuple";
    size_t suffix_len = strlen(suffix);
    if (len < suffix_len || strcmp(coordinate + len - suffix_len, suffix) != 0) {
        g_last_error = TUPLEDNS_ERROR_INVALID_COORDINATE;
        return 0;
    }
    
    /* Check for valid DNS characters */
    for (size_t i = 0; i < len; i++) {
        char c = coordinate[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || c == '.' || c == '-')) {
            g_last_error = TUPLEDNS_ERROR_INVALID_COORDINATE;
            return 0;
        }
    }
    
    return 1;
}

char* tupledns_encode_coordinate(const char* space_type, const char* values[], int value_count) {
    if (!space_type || !values || value_count <= 0) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return NULL;
    }
    
    /* Calculate total length needed */
    size_t total_len = strlen(space_type) + strlen(".tuple") + value_count; /* dots */
    for (int i = 0; i < value_count; i++) {
        if (values[i]) {
            total_len += strlen(values[i]);
        }
    }
    
    if (total_len > TUPLEDNS_MAX_COORDINATE_LENGTH) {
        g_last_error = TUPLEDNS_ERROR_INVALID_COORDINATE;
        return NULL;
    }
    
    char* result = malloc(total_len + 1);
    if (!result) {
        g_last_error = TUPLEDNS_ERROR_MEMORY_ALLOCATION;
        return NULL;
    }
    
    result[0] = '\0';
    
    /* Build coordinate: value1.value2.value3.space_type.tuple */
    for (int i = 0; i < value_count; i++) {
        if (values[i]) {
            strcat(result, values[i]);
            strcat(result, ".");
        }
    }
    strcat(result, space_type);
    strcat(result, ".tuple");
    
    return result;
}

int tupledns_decode_coordinate(const char* coordinate, char** space_type, char*** values, int* value_count) {
    if (!coordinate || !space_type || !values || !value_count) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return TUPLEDNS_ERROR_INVALID_PARAMETER;
    }
    
    if (!tupledns_validate_coordinate(coordinate)) {
        return g_last_error;
    }
    
    /* Remove .tuple suffix */
    size_t coord_len = strlen(coordinate);
    size_t suffix_len = strlen(".tuple");
    char* coord_copy = malloc(coord_len - suffix_len + 1);
    if (!coord_copy) {
        g_last_error = TUPLEDNS_ERROR_MEMORY_ALLOCATION;
        return TUPLEDNS_ERROR_MEMORY_ALLOCATION;
    }
    
    strncpy(coord_copy, coordinate, coord_len - suffix_len);
    coord_copy[coord_len - suffix_len] = '\0';
    
    /* Split by dots */
    char** parts = tupledns_split_string(coord_copy, ".", value_count);
    free(coord_copy);
    
    if (!parts || *value_count < 2) {
        g_last_error = TUPLEDNS_ERROR_INVALID_COORDINATE;
        return TUPLEDNS_ERROR_INVALID_COORDINATE;
    }
    
    /* Last part is space_type */
    *space_type = strdup(parts[*value_count - 1]);
    if (!*space_type) {
        tupledns_free_string_array(parts, *value_count);
        g_last_error = TUPLEDNS_ERROR_MEMORY_ALLOCATION;
        return TUPLEDNS_ERROR_MEMORY_ALLOCATION;
    }
    
    /* Remaining parts are values */
    int num_values = *value_count - 1;
    if (num_values > 0) {
        *values = malloc(num_values * sizeof(char*));
        if (!*values) {
            free(*space_type);
            tupledns_free_string_array(parts, *value_count);
            g_last_error = TUPLEDNS_ERROR_MEMORY_ALLOCATION;
            return TUPLEDNS_ERROR_MEMORY_ALLOCATION;
        }
        
        for (int i = 0; i < num_values; i++) {
            (*values)[i] = strdup(parts[i]);
            if (!(*values)[i]) {
                for (int j = 0; j < i; j++) {
                    free((*values)[j]);
                }
                free(*values);
                free(*space_type);
                tupledns_free_string_array(parts, *value_count);
                g_last_error = TUPLEDNS_ERROR_MEMORY_ALLOCATION;
                return TUPLEDNS_ERROR_MEMORY_ALLOCATION;
            }
        }
    } else {
        *values = NULL;
    }
    
    *value_count = num_values;
    tupledns_free_string_array(parts, *value_count + 1);
    return TUPLEDNS_OK;
}

int tupledns_match_pattern(const char* coordinate, const char* pattern) {
    if (!coordinate || !pattern) {
        return 0;
    }
    
    /* Simple wildcard matching - * matches any single component */
    char** coord_parts;
    char** pattern_parts;
    int coord_count, pattern_count;
    
    coord_parts = tupledns_split_string(coordinate, ".", &coord_count);
    pattern_parts = tupledns_split_string(pattern, ".", &pattern_count);
    
    if (!coord_parts || !pattern_parts || coord_count != pattern_count) {
        tupledns_free_string_array(coord_parts, coord_count);
        tupledns_free_string_array(pattern_parts, pattern_count);
        return 0;
    }
    
    int match = 1;
    for (int i = 0; i < coord_count && match; i++) {
        if (strcmp(pattern_parts[i], "*") != 0 && 
            strcmp(pattern_parts[i], coord_parts[i]) != 0) {
            match = 0;
        }
    }
    
    tupledns_free_string_array(coord_parts, coord_count);
    tupledns_free_string_array(pattern_parts, pattern_count);
    
    return match;
}

/* ========================================================================
 * DNS QUERY FUNCTIONS
 * ======================================================================== */

int tupledns_dns_query_a(const char* hostname, char** ip_address) {
    if (!hostname || !ip_address) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return TUPLEDNS_ERROR_INVALID_PARAMETER;
    }
    
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(hostname, NULL, &hints, &result);
    if (status != 0) {
        g_last_error = TUPLEDNS_ERROR_DNS_QUERY_FAILED;
        return TUPLEDNS_ERROR_DNS_QUERY_FAILED;
    }
    
    char* addr_str = malloc(INET6_ADDRSTRLEN);
    if (!addr_str) {
        freeaddrinfo(result);
        g_last_error = TUPLEDNS_ERROR_MEMORY_ALLOCATION;
        return TUPLEDNS_ERROR_MEMORY_ALLOCATION;
    }
    
    void* addr_ptr;
    if (result->ai_family == AF_INET) {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)result->ai_addr;
        addr_ptr = &(ipv4->sin_addr);
    } else {
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)result->ai_addr;
        addr_ptr = &(ipv6->sin6_addr);
    }
    
    if (!inet_ntop(result->ai_family, addr_ptr, addr_str, INET6_ADDRSTRLEN)) {
        free(addr_str);
        freeaddrinfo(result);
        g_last_error = TUPLEDNS_ERROR_DNS_QUERY_FAILED;
        return TUPLEDNS_ERROR_DNS_QUERY_FAILED;
    }
    
    *ip_address = addr_str;
    freeaddrinfo(result);
    return TUPLEDNS_OK;
}

int tupledns_dns_query_txt(const char* hostname, char*** txt_records, int* record_count) {
    /* Note: This is a simplified implementation. 
     * In practice, you'd want to use a proper DNS library like ldns or c-ares 
     * for full TXT record parsing. */
    
    if (!hostname || !txt_records || !record_count) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return TUPLEDNS_ERROR_INVALID_PARAMETER;
    }
    
    *txt_records = NULL;
    *record_count = 0;
    
    /* For now, return empty result - would need proper DNS library */
    g_last_error = TUPLEDNS_ERROR_NO_RESULTS;
    return TUPLEDNS_ERROR_NO_RESULTS;
}

/* ========================================================================
 * CAPABILITY HANDLING
 * ======================================================================== */

int tupledns_parse_capabilities(const char* txt_record, char*** capabilities, int* capability_count) {
    if (!txt_record || !capabilities || !capability_count) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return TUPLEDNS_ERROR_INVALID_PARAMETER;
    }
    
    /* Look for caps= prefix */
    const char* caps_prefix = "caps=";
    const char* caps_start = strstr(txt_record, caps_prefix);
    if (!caps_start) {
        *capabilities = NULL;
        *capability_count = 0;
        return TUPLEDNS_OK;
    }
    
    caps_start += strlen(caps_prefix);
    
    /* Find end of capabilities (space or end of string) */
    const char* caps_end = strchr(caps_start, ' ');
    if (!caps_end) {
        caps_end = caps_start + strlen(caps_start);
    }
    
    /* Extract capabilities substring */
    size_t caps_len = caps_end - caps_start;
    char* caps_str = malloc(caps_len + 1);
    if (!caps_str) {
        g_last_error = TUPLEDNS_ERROR_MEMORY_ALLOCATION;
        return TUPLEDNS_ERROR_MEMORY_ALLOCATION;
    }
    
    strncpy(caps_str, caps_start, caps_len);
    caps_str[caps_len] = '\0';
    
    /* Split by commas */
    *capabilities = tupledns_split_string(caps_str, ",", capability_count);
    free(caps_str);
    
    if (!*capabilities) {
        *capability_count = 0;
        g_last_error = TUPLEDNS_ERROR_CAPABILITY_PARSE;
        return TUPLEDNS_ERROR_CAPABILITY_PARSE;
    }
    
    return TUPLEDNS_OK;
}

char** tupledns_copy_capabilities(const char* capabilities[], int count) {
    if (!capabilities || count <= 0) {
        return NULL;
    }
    
    char** result = malloc(count * sizeof(char*));
    if (!result) {
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        if (capabilities[i]) {
            result[i] = strdup(capabilities[i]);
            if (!result[i]) {
                tupledns_free_capabilities(result, i);
                return NULL;
            }
        } else {
            result[i] = NULL;
        }
    }
    
    return result;
}

void tupledns_free_capabilities(char** capabilities, int count) {
    tupledns_free_string_array(capabilities, count);
}

int tupledns_has_capability(const tupledns_node_t* node, const char* capability) {
    if (!node || !capability) {
        return 0;
    }
    
    for (int i = 0; i < node->capability_count; i++) {
        if (node->capabilities[i] && strcmp(node->capabilities[i], capability) == 0) {
            return 1;
        }
    }
    
    return 0;
}

/* ========================================================================
 * MEMORY MANAGEMENT
 * ======================================================================== */

void tupledns_free_node(tupledns_node_t* node) {
    if (!node) return;
    
    free(node->coordinate);
    free(node->ip_address);
    tupledns_free_capabilities(node->capabilities, node->capability_count);
    memset(node, 0, sizeof(tupledns_node_t));
}

void tupledns_free_result(tupledns_result_t* result) {
    if (!result) return;
    
    if (result->nodes) {
        for (int i = 0; i < result->node_count; i++) {
            tupledns_free_node(&result->nodes[i]);
        }
        free(result->nodes);
    }
    
    free(result);
}

/* ========================================================================
 * CORE API FUNCTIONS
 * ======================================================================== */

int tupledns_register(const char* coordinate, const char* capabilities[], int ttl) {
    /* This would require DNS server access or dynamic DNS updates */
    /* For now, return success as a placeholder */
    if (!tupledns_validate_coordinate(coordinate)) {
        return g_last_error;
    }
    
    /* TODO: Implement actual DNS registration */
    return TUPLEDNS_OK;
}

int tupledns_register_with_ip(const char* coordinate, const char* ip_address, 
                              const char* capabilities[], int ttl) {
    if (!ip_address) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return TUPLEDNS_ERROR_INVALID_PARAMETER;
    }
    
    return tupledns_register(coordinate, capabilities, ttl);
}

int tupledns_unregister(const char* coordinate) {
    if (!tupledns_validate_coordinate(coordinate)) {
        return g_last_error;
    }
    
    /* TODO: Implement actual DNS unregistration */
    return TUPLEDNS_OK;
}

tupledns_result_t* tupledns_find(const char* pattern) {
    if (!pattern) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return NULL;
    }
    
    tupledns_result_t* result = calloc(1, sizeof(tupledns_result_t));
    if (!result) {
        g_last_error = TUPLEDNS_ERROR_MEMORY_ALLOCATION;
        return NULL;
    }
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    /* TODO: Implement actual DNS queries based on pattern */
    /* For now, return empty result */
    result->nodes = NULL;
    result->node_count = 0;
    result->total_queries = 1;
    result->error = TUPLEDNS_ERROR_NO_RESULTS;
    
    gettimeofday(&end_time, NULL);
    result->query_time = (end_time.tv_sec - start_time.tv_sec) + 
                        (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    
    return result;
}

tupledns_result_t* tupledns_find_with_caps(const char* pattern, const char* required_caps[]) {
    tupledns_result_t* result = tupledns_find(pattern);
    if (!result || !required_caps) {
        return result;
    }
    
    /* TODO: Filter results by capabilities */
    return result;
}

tupledns_result_t* tupledns_find_range(const char* pattern, const tupledns_range_t ranges[], int range_count) {
    if (!pattern || !ranges || range_count <= 0) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return NULL;
    }
    
    /* TODO: Expand ranges into multiple queries and merge results */
    return tupledns_find(pattern);
}

tupledns_result_t* tupledns_search_multi(const char* patterns[], int pattern_count) {
    if (!patterns || pattern_count <= 0) {
        g_last_error = TUPLEDNS_ERROR_INVALID_PARAMETER;
        return NULL;
    }
    
    /* TODO: Execute multiple queries in parallel and merge results */
    return tupledns_find(patterns[0]);
}