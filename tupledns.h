/**
 * TupleDNS v1.0 - Multidimensional Discovery Protocol Using DNS Infrastructure
 * 
 * Portable C99 library for tuple-based service discovery through DNS.
 * Enables discovery of capabilities and services within multidimensional 
 * coordinate spaces using existing DNS infrastructure.
 * 
 * License: MIT
 */

#ifndef TUPLEDNS_H
#define TUPLEDNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <time.h>

/* API Version */
#define TUPLEDNS_VERSION_MAJOR 1
#define TUPLEDNS_VERSION_MINOR 0
#define TUPLEDNS_VERSION_PATCH 0

/* Configuration Constants */
#define TUPLEDNS_MAX_COORDINATE_LENGTH 253  /* Max DNS name length */
#define TUPLEDNS_MAX_CAPABILITIES 32        /* Max capabilities per node */
#define TUPLEDNS_MAX_CAPABILITY_LENGTH 64   /* Max length per capability */
#define TUPLEDNS_MAX_NODES_PER_RESULT 256   /* Max nodes in single result */
#define TUPLEDNS_DEFAULT_TTL 300            /* Default TTL in seconds */
#define TUPLEDNS_DEFAULT_TIMEOUT 5.0        /* Default query timeout */

/* Error Codes */
typedef enum {
    TUPLEDNS_OK = 0,
    TUPLEDNS_ERROR_INVALID_COORDINATE = -1,
    TUPLEDNS_ERROR_DNS_QUERY_FAILED = -2,
    TUPLEDNS_ERROR_MEMORY_ALLOCATION = -3,
    TUPLEDNS_ERROR_INVALID_PARAMETER = -4,
    TUPLEDNS_ERROR_TIMEOUT = -5,
    TUPLEDNS_ERROR_NO_RESULTS = -6,
    TUPLEDNS_ERROR_CAPABILITY_PARSE = -7
} tupledns_error_t;

/* Node Structure */
typedef struct {
    char* coordinate;           /* Full tuple coordinate */
    char* ip_address;          /* IPv4/IPv6 address */
    char** capabilities;       /* Array of capability strings */
    int capability_count;      /* Number of capabilities */
    int ttl;                   /* Time to live */
    time_t last_seen;          /* Last discovery time */
} tupledns_node_t;

/* Query Result Structure */
typedef struct {
    tupledns_node_t* nodes;    /* Array of discovered nodes */
    int node_count;            /* Number of nodes found */
    int total_queries;         /* DNS queries performed */
    double query_time;         /* Total query time (seconds) */
    tupledns_error_t error;    /* Error code if any */
} tupledns_result_t;

/* Range Structure for range queries */
typedef struct {
    char* dimension;           /* Dimension name (e.g., "bpm") */
    int min_value;            /* Minimum value */
    int max_value;            /* Maximum value */
} tupledns_range_t;

/* Configuration Structure */
typedef struct {
    double timeout;           /* Query timeout in seconds */
    int max_concurrent;       /* Max concurrent DNS queries */
    int enable_caching;       /* Enable DNS response caching */
    int cache_ttl;           /* Cache TTL override */
} tupledns_config_t;

/* Library Initialization */
int tupledns_init(const tupledns_config_t* config);
void tupledns_cleanup(void);

/* Registration Functions */
int tupledns_register(const char* coordinate, const char* capabilities[], int ttl);
int tupledns_register_with_ip(const char* coordinate, const char* ip_address, 
                              const char* capabilities[], int ttl);
int tupledns_unregister(const char* coordinate);

/* Discovery Functions */
tupledns_result_t* tupledns_find(const char* pattern);
tupledns_result_t* tupledns_find_with_caps(const char* pattern, const char* required_caps[]);
tupledns_result_t* tupledns_find_range(const char* pattern, const tupledns_range_t ranges[], int range_count);
tupledns_result_t* tupledns_search_multi(const char* patterns[], int pattern_count);

/* Utility Functions */
int tupledns_validate_coordinate(const char* coordinate);
char* tupledns_encode_coordinate(const char* space_type, const char* values[], int value_count);
int tupledns_decode_coordinate(const char* coordinate, char** space_type, char*** values, int* value_count);
int tupledns_match_pattern(const char* coordinate, const char* pattern);
int tupledns_has_capability(const tupledns_node_t* node, const char* capability);

/* Memory Management */
void tupledns_free_result(tupledns_result_t* result);
void tupledns_free_node(tupledns_node_t* node);
char** tupledns_copy_capabilities(const char* capabilities[], int count);
void tupledns_free_capabilities(char** capabilities, int count);

/* Error Handling */
const char* tupledns_error_string(tupledns_error_t error);
tupledns_error_t tupledns_get_last_error(void);

/* Configuration Helpers */
tupledns_config_t tupledns_default_config(void);
int tupledns_set_config(const tupledns_config_t* config);
tupledns_config_t tupledns_get_config(void);

/* String Utilities */
char* tupledns_join_strings(const char* strings[], int count, const char* separator);
char** tupledns_split_string(const char* str, const char* separator, int* count);
void tupledns_free_string_array(char** strings, int count);

/* DNS Helper Functions (Internal, exposed for testing) */
int tupledns_dns_query_a(const char* hostname, char** ip_address);
int tupledns_dns_query_txt(const char* hostname, char*** txt_records, int* record_count);
int tupledns_parse_capabilities(const char* txt_record, char*** capabilities, int* capability_count);

#ifdef __cplusplus
}
#endif

#endif /* TUPLEDNS_H */