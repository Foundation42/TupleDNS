/**
 * TupleDNS Spatial Example
 * 
 * Demonstrates spatial coordinate usage for IoT device discovery
 */

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include "tupledns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* name;
    const char* coordinate;
    const char* capabilities[8];
    const char* description;
} spatial_device_t;

int main() {
    printf("TupleDNS Spatial Discovery Example\n");
    printf("==================================\n\n");
    
    /* Initialize TupleDNS */
    tupledns_config_t config = tupledns_default_config();
    config.timeout = 2.0;  /* Shorter timeout for example */
    
    int result = tupledns_init(&config);
    if (result != TUPLEDNS_OK) {
        printf("Failed to initialize TupleDNS: %s\n", tupledns_error_string(result));
        return 1;
    }
    
    /* Example spatial devices in a building */
    spatial_device_t devices[] = {
        {
            .name = "Coffee Maker",
            .coordinate = "coffee-maker.kitchen.floor-1.building-5.spatial.tuple",
            .capabilities = {"brew", "schedule", "iot", "wifi", NULL},
            .description = "Smart coffee maker in the kitchen"
        },
        {
            .name = "3D Printer",
            .coordinate = "printer-1.lab.floor-2.building-5.spatial.tuple", 
            .capabilities = {"3d-print", "pla", "petg", "wifi", NULL},
            .description = "3D printer in the lab"
        },
        {
            .name = "Smart Light",
            .coordinate = "light-1.office-201.floor-2.building-5.spatial.tuple",
            .capabilities = {"lighting", "dimmer", "color", "iot", NULL},
            .description = "Smart LED light in office 201"
        },
        {
            .name = "Temperature Sensor",
            .coordinate = "temp-1.hallway.floor-1.building-5.spatial.tuple",
            .capabilities = {"temperature", "humidity", "sensor", "iot", NULL},
            .description = "Environmental sensor in hallway"
        },
        {
            .name = "Security Camera",
            .coordinate = "camera-1.entrance.floor-1.building-5.spatial.tuple",
            .capabilities = {"video", "motion", "security", "wifi", NULL},
            .description = "Security camera at entrance"
        }
    };
    
    int num_devices = sizeof(devices) / sizeof(devices[0]);
    
    printf("Example Building Layout:\n");
    printf("========================\n");
    printf("Building 5\n");
    printf("├── Floor 1\n");
    printf("│   ├── Kitchen: Coffee Maker\n");
    printf("│   ├── Hallway: Temperature Sensor\n"); 
    printf("│   └── Entrance: Security Camera\n");
    printf("└── Floor 2\n");
    printf("    ├── Lab: 3D Printer\n");
    printf("    └── Office 201: Smart Light\n\n");
    
    /* Test coordinate validation for spatial devices */
    printf("1. Validating Spatial Coordinates\n");
    printf("----------------------------------\n");
    
    for (int i = 0; i < num_devices; i++) {
        int is_valid = tupledns_validate_coordinate(devices[i].coordinate);
        printf("  %-15s: %-50s [%s]\n", 
               devices[i].name, 
               devices[i].coordinate,
               is_valid ? "Valid" : "Invalid");
    }
    
    /* Demonstrate hierarchical coordinate structure */
    printf("\n2. Coordinate Hierarchy Analysis\n");
    printf("--------------------------------\n");
    
    const char* sample_coord = devices[0].coordinate;
    char* space_type;
    char** values;
    int value_count;
    
    result = tupledns_decode_coordinate(sample_coord, &space_type, &values, &value_count);
    if (result == TUPLEDNS_OK) {
        printf("  Sample coordinate: %s\n", sample_coord);
        printf("  Space type: %s\n", space_type);
        printf("  Hierarchy levels: %d\n", value_count);
        printf("  Path: ");
        for (int i = value_count - 1; i >= 0; i--) {  /* Reverse order for readability */
            printf("%s", values[i]);
            if (i > 0) printf(" → ");
        }
        printf("\n");
        
        free(space_type);
        tupledns_free_string_array(values, value_count);
    }
    
    /* Test different spatial query patterns */
    printf("\n3. Spatial Query Patterns\n");
    printf("-------------------------\n");
    
    struct {
        const char* pattern;
        const char* description;
    } queries[] = {
        {"*.*.*.building-5.spatial.tuple", "All devices in Building 5"},
        {"*.*.floor-1.building-5.spatial.tuple", "All devices on Floor 1"},
        {"*.*.floor-2.building-5.spatial.tuple", "All devices on Floor 2"},
        {"*.kitchen.floor-1.building-5.spatial.tuple", "All devices in Kitchen"},
        {"*.lab.*.building-5.spatial.tuple", "All devices in Lab"},
        {"coffee-maker.*.*.*.spatial.tuple", "Coffee makers anywhere"},
        {"*.office-*.*.building-5.spatial.tuple", "All devices in offices"}
    };
    
    int num_queries = sizeof(queries) / sizeof(queries[0]);
    
    for (int i = 0; i < num_queries; i++) {
        printf("  Query: %s\n", queries[i].description);
        printf("    Pattern: %s\n", queries[i].pattern);
        
        /* Test pattern matching against our example devices */
        int matches = 0;
        for (int j = 0; j < num_devices; j++) {
            if (tupledns_match_pattern(devices[j].coordinate, queries[i].pattern)) {
                if (matches == 0) {
                    printf("    Matches:\n");
                }
                printf("      - %s: %s\n", devices[j].name, devices[j].coordinate);
                matches++;
            }
        }
        if (matches == 0) {
            printf("    Matches: None\n");
        }
        printf("\n");
    }
    
    /* Demonstrate capability-based discovery */
    printf("4. Capability-Based Discovery\n");
    printf("-----------------------------\n");
    
    const char* capability_queries[] = {
        "iot",
        "wifi", 
        "3d-print",
        "sensor",
        "lighting"
    };
    
    int num_cap_queries = sizeof(capability_queries) / sizeof(capability_queries[0]);
    
    for (int i = 0; i < num_cap_queries; i++) {
        printf("  Looking for capability: %s\n", capability_queries[i]);
        
        int found = 0;
        for (int j = 0; j < num_devices; j++) {
            /* Check if device has this capability */
            int has_capability = 0;
            for (int k = 0; devices[j].capabilities[k] != NULL; k++) {
                if (strcmp(devices[j].capabilities[k], capability_queries[i]) == 0) {
                    has_capability = 1;
                    break;
                }
            }
            
            if (has_capability) {
                if (found == 0) {
                    printf("    Found devices:\n");
                }
                printf("      - %s: %s\n", devices[j].name, devices[j].coordinate);
                found++;
            }
        }
        
        if (found == 0) {
            printf("    No devices found with this capability\n");
        }
        printf("\n");
    }
    
    /* Simulate range-based queries (proximity search) */
    printf("5. Proximity-Based Discovery\n");
    printf("----------------------------\n");
    
    struct {
        const char* location;
        const char* search_pattern;
        const char* description;
    } proximity_searches[] = {
        {
            "kitchen.floor-1.building-5",
            "*.floor-1.building-5.spatial.tuple",
            "Devices on same floor as kitchen"
        },
        {
            "office-201.floor-2.building-5", 
            "*.floor-2.building-5.spatial.tuple",
            "Devices on same floor as office 201"
        },
        {
            "lab.floor-2.building-5",
            "*.*.floor-2.building-5.spatial.tuple", 
            "Devices near the lab"
        }
    };
    
    int num_prox = sizeof(proximity_searches) / sizeof(proximity_searches[0]);
    
    for (int i = 0; i < num_prox; i++) {
        printf("  From location: %s\n", proximity_searches[i].location);
        printf("  Search: %s\n", proximity_searches[i].description);
        printf("  Pattern: %s\n", proximity_searches[i].search_pattern);
        
        int found = 0;
        for (int j = 0; j < num_devices; j++) {
            if (tupledns_match_pattern(devices[j].coordinate, proximity_searches[i].search_pattern)) {
                if (found == 0) {
                    printf("    Nearby devices:\n");
                }
                printf("      - %s: %s\n", devices[j].name, devices[j].coordinate);
                found++;
            }
        }
        printf("\n");
    }
    
    /* Test actual DNS discovery (will return empty results) */
    printf("6. Live DNS Discovery Test\n");
    printf("--------------------------\n");
    printf("Note: This would query actual DNS servers if configured.\n\n");
    
    tupledns_result_t* find_result = tupledns_find("*.*.floor-1.building-5.spatial.tuple");
    if (find_result) {
        printf("  DNS Query Results:\n");
        printf("    Found nodes: %d\n", find_result->node_count);
        printf("    Query time: %.3f seconds\n", find_result->query_time);
        printf("    Total queries: %d\n", find_result->total_queries);
        printf("    Status: %s\n", tupledns_error_string(find_result->error));
        
        tupledns_free_result(find_result);
    }
    
    /* Cleanup */
    tupledns_cleanup();
    
    printf("\nSpatial discovery example completed!\n");
    printf("This demonstrates how TupleDNS can organize IoT devices\n");
    printf("in a hierarchical spatial coordinate system for easy discovery.\n");
    
    return 0;
}