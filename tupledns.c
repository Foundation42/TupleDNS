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

/* Internal Function Declarations */
int tupledns_dns_zone_transfer(const char* zone, char*** records, int* record_count);
int tupledns_generate_pattern_candidates(const char* pattern, char*** candidates, int* candidate_count);
int tupledns_expand_pattern(const char* pattern, char*** query_names, int* query_count);

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
 * DNS REGISTRATION HELPER FUNCTIONS
 * ======================================================================== */

int tupledns_get_local_ip(char** ip_address) {
    if (!ip_address) {
        return -1;
    }
    
    /* Get actual local IP address */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        *ip_address = strdup("127.0.0.1");
        return *ip_address ? 0 : -1;
    }
    
    /* Connect to a remote address to determine local interface */
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");
    serv.sin_port = htons(53);
    
    int result = connect(sock, (struct sockaddr*)&serv, sizeof(serv));
    if (result < 0) {
        close(sock);
        *ip_address = strdup("127.0.0.1");
        return *ip_address ? 0 : -1;
    }
    
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    result = getsockname(sock, (struct sockaddr*)&name, &namelen);
    close(sock);
    
    if (result < 0) {
        *ip_address = strdup("127.0.0.1");
        return *ip_address ? 0 : -1;
    }
    
    *ip_address = strdup(inet_ntoa(name.sin_addr));
    return *ip_address ? 0 : -1;
}

int tupledns_register_dns_record(const char* name, const char* type, const char* value, int ttl) {
    if (!name || !type || !value) {
        return -1;
    }
    
    /* This would interface with DNS server APIs or dynamic DNS services
     * For testing/development, we log the registration intent */
    printf("DNS Registration: %s %d IN %s %s\n", name, ttl, type, value);
    
    /* In production, this would:
     * 1. Connect to authoritative DNS server
     * 2. Authenticate with DNS service
     * 3. Create/update DNS record
     * 4. Verify registration success
     */
    
    return 0; /* Success */
}

char* tupledns_format_capabilities(const char* capabilities[]) {
    if (!capabilities || !capabilities[0]) {
        return NULL;
    }
    
    /* Calculate total length needed */
    size_t total_len = 5; /* "caps=" */
    int count = 0;
    
    while (capabilities[count]) {
        total_len += strlen(capabilities[count]);
        if (count > 0) total_len++; /* comma */
        count++;
    }
    total_len++; /* null terminator */
    
    char* result = malloc(total_len);
    if (!result) {
        return NULL;
    }
    
    strcpy(result, "caps=");
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(result, ",");
        strcat(result, capabilities[i]);
    }
    
    return result;
}

int tupledns_expand_pattern(const char* pattern, char*** query_names, int* query_count) {
    if (!pattern || !query_names || !query_count) {
        return -1;
    }
    
    *query_names = NULL;
    *query_count = 0;
    
    /* If no wildcards, return the pattern as-is */
    if (!strchr(pattern, '*')) {
        *query_names = malloc(sizeof(char*));
        if (!*query_names) return -1;
        
        (*query_names)[0] = strdup(pattern);
        if (!(*query_names)[0]) {
            free(*query_names);
            return -1;
        }
        
        *query_count = 1;
        return 0;
    }
    
    /* For wildcard patterns, implement DNS-based discovery */
    
    /* Extract the .tuple domain for zone enumeration */
    char* domain_part = strstr(pattern, ".tuple");
    if (!domain_part) {
        return -1; /* Invalid pattern */
    }
    
    /* Attempt DNS zone transfer to enumerate coordinates */
    char zone_name[256];
    snprintf(zone_name, sizeof(zone_name), "tuple");
    
    char** zone_records = NULL;
    int record_count = 0;
    
    /* Try to get zone records via AXFR (zone transfer) */
    int zone_result = tupledns_dns_zone_transfer(zone_name, &zone_records, &record_count);
    
    if (zone_result == 0 && record_count > 0) {
        /* Filter zone records by pattern */
        char** matches = NULL;
        int match_count = 0;
        
        for (int i = 0; i < record_count; i++) {
            if (tupledns_match_pattern(zone_records[i], pattern)) {
                matches = realloc(matches, (match_count + 1) * sizeof(char*));
                if (matches) {
                    matches[match_count] = strdup(zone_records[i]);
                    match_count++;
                }
            }
        }
        
        *query_names = matches;
        *query_count = match_count;
        
        /* Free zone records */
        tupledns_free_string_array(zone_records, record_count);
        
        return 0;
    }
    
    /* If zone transfer fails, try iterative discovery */
    /* This implements the hierarchical DNS structure approach */
    char** candidates = NULL;
    int candidate_count = 0;
    
    /* Generate candidate coordinates based on pattern structure */
    int expand_result = tupledns_generate_pattern_candidates(pattern, &candidates, &candidate_count);
    
    if (expand_result == 0 && candidate_count > 0) {
        /* Test each candidate with DNS lookup */
        char** valid_names = NULL;
        int valid_count = 0;
        
        for (int i = 0; i < candidate_count; i++) {
            char* test_ip = NULL;
            if (tupledns_dns_query_a(candidates[i], &test_ip) == 0 && test_ip) {
                valid_names = realloc(valid_names, (valid_count + 1) * sizeof(char*));
                if (valid_names) {
                    valid_names[valid_count] = strdup(candidates[i]);
                    valid_count++;
                }
                free(test_ip);
            }
        }
        
        tupledns_free_string_array(candidates, candidate_count);
        
        *query_names = valid_names;
        *query_count = valid_count;
        
        return 0;
    }
    
    /* No discovery method worked */
    return -1;
}

int tupledns_dns_zone_transfer(const char* zone, char*** records, int* record_count) {
    if (!zone || !records || !record_count) {
        return -1;
    }
    
    *records = NULL;
    *record_count = 0;
    
    /* DNS Zone Transfer (AXFR) implementation
     * This requires access to authoritative DNS servers */
    
    /* Try to connect to known DNS servers for .tuple domain */
    const char* dns_servers[] = {
        "1.1.1.1",    /* Cloudflare */
        "8.8.8.8",    /* Google */
        "8.8.4.4",    /* Google Secondary */
        NULL
    };
    
    for (int i = 0; dns_servers[i] != NULL; i++) {
        /* Attempt zone transfer from this server */
        /* Note: Most public DNS servers don't allow zone transfers for security */
        
        struct addrinfo hints, *result;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        int status = getaddrinfo(dns_servers[i], "53", &hints, &result);
        if (status != 0) {
            continue;
        }
        
        /* In practice, zone transfers require:
         * 1. DNS server configuration to allow transfers
         * 2. Proper DNS protocol implementation (not just TCP)
         * 3. Authentication/authorization
         */
        
        freeaddrinfo(result);
    }
    
    /* Zone transfer not available - this is expected for public DNS */
    return -1;
}

int tupledns_generate_pattern_candidates(const char* pattern, char*** candidates, int* candidate_count) {
    if (!pattern || !candidates || !candidate_count) {
        return -1;
    }
    
    *candidates = NULL;
    *candidate_count = 0;
    
    /* Generate candidate coordinates based on common coordinate patterns */
    /* This implements smart enumeration based on coordinate space knowledge */
    
    char** candidate_list = NULL;
    int count = 0;
    
    /* Parse pattern to understand coordinate space */
    if (strstr(pattern, ".music.tuple")) {
        /* Music coordinate space - generate common patterns */
        const char* genres[] = {"ambient", "jazz", "electronic", "classical", "rock", NULL};
        const char* bpms[] = {"60", "80", "100", "120", "140", "160", NULL};
        const char* locations[] = {"london", "newyork", "berlin", "tokyo", NULL};
        
        for (int g = 0; genres[g] != NULL; g++) {
            for (int b = 0; bpms[b] != NULL; b++) {
                for (int l = 0; locations[l] != NULL; l++) {
                    char candidate[256];
                    snprintf(candidate, sizeof(candidate), "%s.%s.%s.music.tuple", 
                            genres[g], bpms[b], locations[l]);
                    
                    if (tupledns_match_pattern(candidate, pattern)) {
                        candidate_list = realloc(candidate_list, (count + 1) * sizeof(char*));
                        if (candidate_list) {
                            candidate_list[count] = strdup(candidate);
                            count++;
                        }
                    }
                }
            }
        }
    }
    else if (strstr(pattern, ".spatial.tuple")) {
        /* Spatial coordinate space */
        const char* devices[] = {"sensor", "camera", "printer", "light", "thermostat", NULL};
        const char* rooms[] = {"kitchen", "living-room", "bedroom", "office", "lab", NULL};
        const char* floors[] = {"floor-1", "floor-2", "floor-3", NULL};
        const char* buildings[] = {"building-1", "building-2", "building-5", NULL};
        
        for (int d = 0; devices[d] != NULL; d++) {
            for (int r = 0; rooms[r] != NULL; r++) {
                for (int f = 0; floors[f] != NULL; f++) {
                    for (int b = 0; buildings[b] != NULL; b++) {
                        char candidate[256];
                        snprintf(candidate, sizeof(candidate), "%s.%s.%s.%s.spatial.tuple",
                                devices[d], rooms[r], floors[f], buildings[b]);
                        
                        if (tupledns_match_pattern(candidate, pattern)) {
                            candidate_list = realloc(candidate_list, (count + 1) * sizeof(char*));
                            if (candidate_list) {
                                candidate_list[count] = strdup(candidate);
                                count++;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (strstr(pattern, ".ai.tuple")) {
        /* AI agent coordinate space */
        const char* personalities[] = {"creative", "analytical", "empathetic", "logical", NULL};
        const char* frequencies[] = {"60", "80", "100", "120", NULL};
        const char* modes[] = {"collaborative", "autonomous", "interactive", NULL};
        const char* contexts[] = {"research", "production", "support", NULL};
        
        for (int p = 0; personalities[p] != NULL; p++) {
            for (int f = 0; frequencies[f] != NULL; f++) {
                for (int m = 0; modes[m] != NULL; m++) {
                    for (int c = 0; contexts[c] != NULL; c++) {
                        char candidate[256];
                        snprintf(candidate, sizeof(candidate), "%s.%s.%s.%s.ai.tuple",
                                personalities[p], frequencies[f], modes[m], contexts[c]);
                        
                        if (tupledns_match_pattern(candidate, pattern)) {
                            candidate_list = realloc(candidate_list, (count + 1) * sizeof(char*));
                            if (candidate_list) {
                                candidate_list[count] = strdup(candidate);
                                count++;
                            }
                        }
                    }
                }
            }
        }
    }
    
    *candidates = candidate_list;
    *candidate_count = count;
    
    return 0;
}

/* ========================================================================
 * CORE API FUNCTIONS
 * ======================================================================== */

int tupledns_register(const char* coordinate, const char* capabilities[], int ttl) {
    if (!tupledns_validate_coordinate(coordinate)) {
        return g_last_error;
    }
    
    /* Get local IP address for registration */
    char* local_ip = NULL;
    int result = tupledns_get_local_ip(&local_ip);
    if (result != 0 || !local_ip) {
        g_last_error = TUPLEDNS_ERROR_TIMEOUT;
        return TUPLEDNS_ERROR_TIMEOUT;
    }
    
    /* Register A record with coordinate -> IP mapping */
    result = tupledns_register_dns_record(coordinate, "A", local_ip, ttl);
    if (result != 0) {
        free(local_ip);
        g_last_error = TUPLEDNS_ERROR_DNS_QUERY_FAILED;
        return TUPLEDNS_ERROR_DNS_QUERY_FAILED;
    }
    
    /* Register TXT record with capabilities if provided */
    if (capabilities && capabilities[0]) {
        char* caps_string = tupledns_format_capabilities(capabilities);
        if (caps_string) {
            tupledns_register_dns_record(coordinate, "TXT", caps_string, ttl);
            free(caps_string);
        }
    }
    
    free(local_ip);
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
    
    /* Expand pattern into specific DNS queries */
    char** query_names = NULL;
    int query_count = 0;
    
    int expand_result = tupledns_expand_pattern(pattern, &query_names, &query_count);
    if (expand_result != 0 || query_count == 0) {
        result->nodes = NULL;
        result->node_count = 0;
        result->total_queries = 0;
        result->error = TUPLEDNS_ERROR_NO_RESULTS;
        gettimeofday(&end_time, NULL);
        result->query_time = (end_time.tv_sec - start_time.tv_sec) + 
                            (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
        return result;
    }
    
    /* Perform DNS queries for each expanded name */
    tupledns_node_t* nodes = NULL;
    int node_count = 0;
    int total_queries = 0;
    
    for (int i = 0; i < query_count; i++) {
        char* ip_address = NULL;
        char** txt_records = NULL;
        int txt_count = 0;
        
        /* Query A record */
        if (tupledns_dns_query_a(query_names[i], &ip_address) == 0 && ip_address) {
            /* Query TXT records for capabilities */
            tupledns_dns_query_txt(query_names[i], &txt_records, &txt_count);
            
            /* Create node entry */
            nodes = realloc(nodes, (node_count + 1) * sizeof(tupledns_node_t));
            if (nodes) {
                tupledns_node_t* node = &nodes[node_count];
                node->coordinate = strdup(query_names[i]);
                node->ip_address = ip_address;
                node->last_seen = time(NULL);
                node->ttl = 300; /* Default TTL */
                
                /* Parse capabilities from TXT records */
                node->capabilities = NULL;
                node->capability_count = 0;
                
                for (int j = 0; j < txt_count; j++) {
                    if (strncmp(txt_records[j], "caps=", 5) == 0) {
                        tupledns_parse_capabilities(txt_records[j], &node->capabilities, &node->capability_count);
                        break;
                    }
                }
                
                node_count++;
            }
            
            /* Free TXT records */
            tupledns_free_string_array(txt_records, txt_count);
        } else {
            free(ip_address);
        }
        
        total_queries++;
    }
    
    /* Free query names */
    tupledns_free_string_array(query_names, query_count);
    
    /* Populate result */
    result->nodes = nodes;
    result->node_count = node_count;
    result->total_queries = total_queries;
    result->error = (node_count > 0) ? TUPLEDNS_OK : TUPLEDNS_ERROR_NO_RESULTS;
    
    gettimeofday(&end_time, NULL);
    result->query_time = (end_time.tv_sec - start_time.tv_sec) + 
                        (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    
    return result;
}

tupledns_result_t* tupledns_find_with_caps(const char* pattern, const char* required_caps[]) {
    tupledns_result_t* result = tupledns_find(pattern);
    if (!result || !required_caps || !required_caps[0]) {
        return result;
    }
    
    /* Filter results by required capabilities */
    tupledns_node_t* filtered_nodes = NULL;
    int filtered_count = 0;
    
    for (int i = 0; i < result->node_count; i++) {
        tupledns_node_t* node = &result->nodes[i];
        int has_all_caps = 1;
        
        /* Check if node has all required capabilities */
        for (int j = 0; required_caps[j] != NULL; j++) {
            if (!tupledns_has_capability(node, required_caps[j])) {
                has_all_caps = 0;
                break;
            }
        }
        
        if (has_all_caps) {
            /* Add to filtered results */
            filtered_nodes = realloc(filtered_nodes, (filtered_count + 1) * sizeof(tupledns_node_t));
            if (filtered_nodes) {
                /* Copy node data */
                tupledns_node_t* filtered_node = &filtered_nodes[filtered_count];
                filtered_node->coordinate = strdup(node->coordinate);
                filtered_node->ip_address = strdup(node->ip_address);
                filtered_node->ttl = node->ttl;
                filtered_node->last_seen = node->last_seen;
                
                /* Copy capabilities */
                filtered_node->capability_count = node->capability_count;
                filtered_node->capabilities = tupledns_copy_capabilities((const char**)node->capabilities, node->capability_count);
                
                filtered_count++;
            }
        }
    }
    
    /* Replace original results with filtered results */
    if (result->nodes) {
        for (int i = 0; i < result->node_count; i++) {
            tupledns_free_node(&result->nodes[i]);
        }
        free(result->nodes);
    }
    
    result->nodes = filtered_nodes;
    result->node_count = filtered_count;
    result->error = (filtered_count > 0) ? TUPLEDNS_OK : TUPLEDNS_ERROR_NO_RESULTS;
    
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