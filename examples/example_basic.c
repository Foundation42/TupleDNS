/**
 * TupleDNS Basic Example
 * 
 * Demonstrates basic registration and discovery functionality
 */

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include "tupledns.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("TupleDNS Basic Example\n");
    printf("=====================\n\n");
    
    /* Initialize TupleDNS with default configuration */
    tupledns_config_t config = tupledns_default_config();
    int result = tupledns_init(&config);
    if (result != TUPLEDNS_OK) {
        printf("Failed to initialize TupleDNS: %s\n", tupledns_error_string(result));
        return 1;
    }
    
    printf("TupleDNS initialized successfully\n");
    
    /* Test coordinate validation */
    printf("\n1. Testing Coordinate Validation\n");
    printf("---------------------------------\n");
    
    const char* test_coordinates[] = {
        "ambient.120.experimental.music.tuple",
        "london.uk.europe.spatial.tuple", 
        "14.30.24.06.2025.time.tuple",
        "coffee-maker.kitchen.home.device.tuple",
        "invalid-coordinate",
        "too.many.parts.here.and.more.and.more.parts.spatial.tuple",
        ""
    };
    
    int num_coords = sizeof(test_coordinates) / sizeof(test_coordinates[0]);
    for (int i = 0; i < num_coords; i++) {
        int is_valid = tupledns_validate_coordinate(test_coordinates[i]);
        printf("  %-50s: %s\n", test_coordinates[i], is_valid ? "Valid" : "Invalid");
    }
    
    /* Test coordinate encoding */
    printf("\n2. Testing Coordinate Encoding\n");
    printf("------------------------------\n");
    
    const char* space_type = "music";
    const char* values[] = {"ambient", "120", "experimental"};
    int value_count = 3;
    
    char* encoded = tupledns_encode_coordinate(space_type, values, value_count);
    if (encoded) {
        printf("  Encoded coordinate: %s\n", encoded);
        
        /* Test decoding */
        char* decoded_space_type;
        char** decoded_values;
        int decoded_count;
        
        int decode_result = tupledns_decode_coordinate(encoded, &decoded_space_type, 
                                                     &decoded_values, &decoded_count);
        if (decode_result == TUPLEDNS_OK) {
            printf("  Decoded space type: %s\n", decoded_space_type);
            printf("  Decoded values: ");
            for (int i = 0; i < decoded_count; i++) {
                printf("%s", decoded_values[i]);
                if (i < decoded_count - 1) printf(".");
            }
            printf("\n");
            
            /* Clean up */
            free(decoded_space_type);
            tupledns_free_string_array(decoded_values, decoded_count);
        } else {
            printf("  Failed to decode coordinate: %s\n", tupledns_error_string(decode_result));
        }
        
        free(encoded);
    } else {
        printf("  Failed to encode coordinate\n");
    }
    
    /* Test pattern matching */
    printf("\n3. Testing Pattern Matching\n");
    printf("---------------------------\n");
    
    const char* coordinate = "ambient.120.experimental.music.tuple";
    const char* patterns[] = {
        "ambient.120.experimental.music.tuple",  /* exact match */
        "*.120.experimental.music.tuple",        /* wildcard genre */
        "ambient.*.experimental.music.tuple",    /* wildcard bpm */
        "ambient.120.*.music.tuple",             /* wildcard style */
        "*.*.*.music.tuple",                     /* all music */
        "jazz.120.experimental.music.tuple",     /* no match */
        "ambient.120.experimental.spatial.tuple" /* wrong space */
    };
    
    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    for (int i = 0; i < num_patterns; i++) {
        int matches = tupledns_match_pattern(coordinate, patterns[i]);
        printf("  %-40s: %s\n", patterns[i], matches ? "Match" : "No match");
    }
    
    /* Test registration (will not actually register without DNS server) */
    printf("\n4. Testing Registration\n");
    printf("-----------------------\n");
    
    const char* capabilities[] = {"midi-in", "midi-out", "real-time", "generative", NULL};
    result = tupledns_register(coordinate, capabilities, 300);
    printf("  Registration result: %s\n", tupledns_error_string(result));
    
    /* Test discovery (will return empty results without actual DNS data) */
    printf("\n5. Testing Discovery\n");
    printf("--------------------\n");
    
    tupledns_result_t* find_result = tupledns_find("*.120.*.music.tuple");
    if (find_result) {
        printf("  Found %d nodes\n", find_result->node_count);
        printf("  Query time: %.3f seconds\n", find_result->query_time);
        printf("  Total queries: %d\n", find_result->total_queries);
        printf("  Result: %s\n", tupledns_error_string(find_result->error));
        
        /* Display found nodes */
        for (int i = 0; i < find_result->node_count; i++) {
            tupledns_node_t* node = &find_result->nodes[i];
            printf("    Node %d: %s @ %s\n", i+1, node->coordinate, node->ip_address);
            printf("    Capabilities: ");
            for (int j = 0; j < node->capability_count; j++) {
                printf("%s", node->capabilities[j]);
                if (j < node->capability_count - 1) printf(", ");
            }
            printf("\n");
        }
        
        tupledns_free_result(find_result);
    } else {
        printf("  Discovery failed: %s\n", tupledns_error_string(tupledns_get_last_error()));
    }
    
    /* Test capability parsing */
    printf("\n6. Testing Capability Parsing\n");
    printf("------------------------------\n");
    
    const char* txt_record = "caps=midi-in,midi-out,real-time version=1.0";
    char** parsed_caps;
    int cap_count;
    
    result = tupledns_parse_capabilities(txt_record, &parsed_caps, &cap_count);
    if (result == TUPLEDNS_OK) {
        printf("  Parsed %d capabilities: ", cap_count);
        for (int i = 0; i < cap_count; i++) {
            printf("%s", parsed_caps[i]);
            if (i < cap_count - 1) printf(", ");
        }
        printf("\n");
        
        tupledns_free_capabilities(parsed_caps, cap_count);
    } else {
        printf("  Capability parsing failed: %s\n", tupledns_error_string(result));
    }
    
    /* Cleanup */
    printf("\n7. Cleanup\n");
    printf("----------\n");
    tupledns_cleanup();
    printf("  TupleDNS cleanup completed\n");
    
    printf("\nBasic example completed successfully!\n");
    printf("Note: Actual DNS registration/discovery requires a configured DNS server.\n");
    
    return 0;
}