# TupleDNS C API Reference

Complete reference for the TupleDNS C library.

## Header Files

```c
#include "tupledns.h"
```

## Core Data Structures

### tupledns_node_t
```c
typedef struct {
    char coordinate[256];
    char ip_address[46];
    char* capabilities[32];
    int capability_count;
    int ttl;
    time_t last_seen;
} tupledns_node_t;
```

### tupledns_result_t
```c
typedef struct {
    tupledns_node_t* nodes;
    int node_count;
    int total_queries;
    double query_time;
} tupledns_result_t;
```

### tupledns_range_t
```c
typedef struct {
    char dimension[64];
    char** values;
    int value_count;
    int min_numeric;
    int max_numeric;
} tupledns_range_t;
```

## Initialization Functions

### tupledns_init()
```c
int tupledns_init(const char* config_file);
```
Initialize the TupleDNS library.

**Parameters:**
- `config_file`: Path to configuration file (NULL for defaults)

**Returns:** 0 on success, negative on error

### tupledns_cleanup()
```c
void tupledns_cleanup(void);
```
Clean up library resources. Call before program exit.

## Registration Functions

### tupledns_register()
```c
int tupledns_register(const char* coordinate, 
                     const char** capabilities, 
                     int ttl);
```
Register a node at the specified coordinate.

**Parameters:**
- `coordinate`: Tuple coordinate (e.g., "ambient.120.london.music.tuple")
- `capabilities`: Array of capability strings, NULL-terminated
- `ttl`: Time-to-live in seconds

**Returns:** 0 on success, negative on error

### tupledns_unregister()
```c
int tupledns_unregister(const char* coordinate);
```
Remove registration for the specified coordinate.

## Discovery Functions

### tupledns_find()
```c
tupledns_result_t* tupledns_find(const char* pattern);
```
Find nodes matching the specified pattern.

**Parameters:**
- `pattern`: Search pattern with wildcards (e.g., "*.120.*.music.tuple")

**Returns:** Result structure or NULL on error

### tupledns_find_with_caps()
```c
tupledns_result_t* tupledns_find_with_caps(const char* pattern,
                                          const char* required_caps[]);
```
Find nodes matching pattern and capabilities.

### tupledns_find_range()
```c
tupledns_result_t* tupledns_find_range(const char* pattern,
                                      tupledns_range_t* ranges,
                                      int range_count);
```
Find nodes within specified dimensional ranges.

## Utility Functions

### tupledns_validate_coordinate()
```c
int tupledns_validate_coordinate(const char* coordinate);
```
Validate coordinate format.

**Returns:** 1 if valid, 0 if invalid

### tupledns_encode_coordinate()
```c
char* tupledns_encode_coordinate(const char* space_type, const char* values[], int value_count);
```
Build coordinate from space type and component values.

**Parameters:**
- `space_type`: The coordinate space type (e.g., "music", "spatial")
- `values`: Array of coordinate values
- `value_count`: Number of values in the array

**Returns:** Allocated coordinate string (caller must free)

### tupledns_decode_coordinate()
```c
int tupledns_decode_coordinate(const char* coordinate, char** space_type, char*** values, int* value_count);
```
Split coordinate into space type and component values.

**Parameters:**
- `coordinate`: The coordinate string to decode
- `space_type`: Output pointer for space type (caller must free)
- `values`: Output pointer for values array (caller must free with tupledns_free_string_array)
- `value_count`: Output pointer for number of values

**Returns:** 0 on success, negative on error

### tupledns_match_pattern()
```c
int tupledns_match_pattern(const char* coordinate, const char* pattern);
```
Test if coordinate matches pattern.

**Returns:** 1 if match, 0 if no match

## Memory Management

### tupledns_free_result()
```c
void tupledns_free_result(tupledns_result_t* result);
```
Free memory allocated by discovery functions.

### tupledns_free_string_array()
```c
void tupledns_free_string_array(char** strings, int count);
```
Free string array returned by decode and other functions.

## Error Codes

```c
#define TUPLEDNS_OK                 0
#define TUPLEDNS_ERROR_INVALID_ARG -1
#define TUPLEDNS_ERROR_DNS_QUERY   -2
#define TUPLEDNS_ERROR_MEMORY      -3
#define TUPLEDNS_ERROR_NETWORK     -4
#define TUPLEDNS_ERROR_TIMEOUT     -5
```

## Thread Safety

The TupleDNS library is thread-safe for read operations after initialization. Registration and unregistration operations should be synchronized by the application.

## Example Usage

See [examples/](../examples/) for complete working examples.