/**
 * TupleDNS Music Collaboration Example
 * 
 * Demonstrates musical coordinate system for creative collaboration
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
    const char* location;
} music_node_t;

int main() {
    printf("TupleDNS Music Collaboration Example\n");
    printf("====================================\n\n");
    
    /* Initialize TupleDNS */
    tupledns_config_t config = tupledns_default_config();
    int result = tupledns_init(&config);
    if (result != TUPLEDNS_OK) {
        printf("Failed to initialize TupleDNS: %s\n", tupledns_error_string(result));
        return 1;
    }
    
    /* Example music nodes across different musical dimensions */
    music_node_t music_nodes[] = {
        {
            .name = "Ambient Studio London",
            .coordinate = "ambient.120.4-4.c-major.london.music.tuple",
            .capabilities = {"midi", "real-time", "generative", "collaborative", NULL},
            .description = "Ambient music studio specializing in generative compositions",
            .location = "London, UK"
        },
        {
            .name = "Jazz Ensemble NYC",
            .coordinate = "jazz.140.4-4.bb-major.newyork.music.tuple",
            .capabilities = {"midi", "live-recording", "improvisation", "collaborative", NULL},
            .description = "Jazz ensemble available for remote collaboration",
            .location = "New York, USA"
        },
        {
            .name = "Electronic Producer",
            .coordinate = "electronic.128.4-4.a-minor.berlin.music.tuple",
            .capabilities = {"midi", "synthesis", "real-time", "remix", NULL},
            .description = "Electronic music producer with live synthesis setup",
            .location = "Berlin, Germany"
        },
        {
            .name = "Ambient Drone Project",
            .coordinate = "ambient.80.7-8.d-minor.portland.music.tuple",
            .capabilities = {"midi", "drone", "atmospheric", "collaborative", NULL},
            .description = "Slow ambient drone music project",
            .location = "Portland, USA"
        },
        {
            .name = "Classical Chamber",
            .coordinate = "classical.120.4-4.g-major.vienna.music.tuple",
            .capabilities = {"acoustic", "chamber", "notation", "recording", NULL},
            .description = "Chamber music ensemble",
            .location = "Vienna, Austria"
        },
        {
            .name = "Experimental Noise",
            .coordinate = "experimental.160.7-8.atonal.tokyo.music.tuple",
            .capabilities = {"midi", "noise", "experimental", "avant-garde", NULL},
            .description = "Experimental noise and sound art collective",
            .location = "Tokyo, Japan"
        }
    };
    
    int num_nodes = sizeof(music_nodes) / sizeof(music_nodes[0]);
    
    printf("Musical Collaboration Network:\n");
    printf("==============================\n");
    for (int i = 0; i < num_nodes; i++) {
        printf("%d. %s (%s)\n", i+1, music_nodes[i].name, music_nodes[i].location);
        printf("   %s\n", music_nodes[i].description);
        printf("   Coordinate: %s\n", music_nodes[i].coordinate);
        printf("   Capabilities: ");
        for (int j = 0; music_nodes[i].capabilities[j] != NULL; j++) {
            printf("%s", music_nodes[i].capabilities[j]);
            if (music_nodes[i].capabilities[j+1] != NULL) printf(", ");
        }
        printf("\n\n");
    }
    
    /* Test musical coordinate analysis */
    printf("1. Musical Coordinate Analysis\n");
    printf("------------------------------\n");
    
    const char* sample_coord = music_nodes[0].coordinate;
    char* space_type;
    char** values;
    int value_count;
    
    result = tupledns_decode_coordinate(sample_coord, &space_type, &values, &value_count);
    if (result == TUPLEDNS_OK) {
        printf("  Sample coordinate: %s\n", sample_coord);
        printf("  Musical dimensions:\n");
        if (value_count >= 1) printf("    Genre: %s\n", values[0]);
        if (value_count >= 2) printf("    BPM: %s\n", values[1]);
        if (value_count >= 3) printf("    Time Signature: %s\n", values[2]);
        if (value_count >= 4) printf("    Key: %s\n", values[3]);
        if (value_count >= 5) printf("    Location: %s\n", values[4]);
        
        free(space_type);
        tupledns_free_string_array(values, value_count);
    }
    
    /* Genre-based discovery */
    printf("\n2. Genre-Based Discovery\n");
    printf("------------------------\n");
    
    const char* genre_queries[] = {
        "ambient",
        "jazz", 
        "electronic",
        "classical",
        "experimental"
    };
    
    int num_genres = sizeof(genre_queries) / sizeof(genre_queries[0]);
    
    for (int i = 0; i < num_genres; i++) {
        printf("  Genre: %s\n", genre_queries[i]);
        
        char pattern[256];
        snprintf(pattern, sizeof(pattern), "%s.*.*.*.*.music.tuple", genre_queries[i]);
        
        int found = 0;
        for (int j = 0; j < num_nodes; j++) {
            if (tupledns_match_pattern(music_nodes[j].coordinate, pattern)) {
                if (found == 0) {
                    printf("    Artists/Studios:\n");
                }
                printf("      - %s: %s\n", music_nodes[j].name, music_nodes[j].coordinate);
                found++;
            }
        }
        if (found == 0) {
            printf("    No matches found\n");
        }
        printf("\n");
    }
    
    /* Tempo-based discovery */
    printf("3. Tempo-Based Discovery\n");
    printf("------------------------\n");
    
    struct {
        int min_bpm;
        int max_bpm;
        const char* description;
    } tempo_ranges[] = {
        {60, 100, "Slow tempo (60-100 BPM)"},
        {100, 130, "Medium tempo (100-130 BPM)"},
        {130, 180, "Fast tempo (130-180 BPM)"}
    };
    
    int num_tempo_ranges = sizeof(tempo_ranges) / sizeof(tempo_ranges[0]);
    
    for (int i = 0; i < num_tempo_ranges; i++) {
        printf("  %s\n", tempo_ranges[i].description);
        
        int found = 0;
        for (int j = 0; j < num_nodes; j++) {
            /* Extract BPM from coordinate */
            char* space_type;
            char** values;
            int value_count;
            
            result = tupledns_decode_coordinate(music_nodes[j].coordinate, &space_type, &values, &value_count);
            if (result == TUPLEDNS_OK && value_count >= 2) {
                int bpm = atoi(values[1]);
                if (bpm >= tempo_ranges[i].min_bpm && bpm <= tempo_ranges[i].max_bpm) {
                    if (found == 0) {
                        printf("    Matching artists:\n");
                    }
                    printf("      - %s (%d BPM): %s\n", music_nodes[j].name, bpm, music_nodes[j].coordinate);
                    found++;
                }
                
                free(space_type);
                tupledns_free_string_array(values, value_count);
            }
        }
        if (found == 0) {
            printf("    No matches found\n");
        }
        printf("\n");
    }
    
    /* Key-based discovery */
    printf("4. Musical Key Discovery\n");
    printf("------------------------\n");
    
    const char* key_families[] = {
        "major",
        "minor"
    };
    
    for (int i = 0; i < 2; i++) {
        printf("  %s keys:\n", key_families[i]);
        
        char pattern[256];
        snprintf(pattern, sizeof(pattern), "*.*.*.%s.*.music.tuple", key_families[i]);
        
        int found = 0;
        for (int j = 0; j < num_nodes; j++) {
            if (tupledns_match_pattern(music_nodes[j].coordinate, pattern)) {
                if (found == 0) {
                    printf("    Artists:\n");
                }
                printf("      - %s: %s\n", music_nodes[j].name, music_nodes[j].coordinate);
                found++;
            }
        }
        if (found == 0) {
            printf("    No matches found\n");
        }
        printf("\n");
    }
    
    /* Collaboration matching */
    printf("5. Collaboration Matching\n");
    printf("-------------------------\n");
    printf("Finding potential collaborators for the Ambient Studio London:\n\n");
    
    const char* source_coord = "ambient.120.4-4.c-major.london.music.tuple";
    
    struct {
        const char* pattern;
        const char* description;
        int priority;
    } collab_queries[] = {
        {"ambient.*.*.*.*.music.tuple", "Same genre (ambient)", 1},
        {"*.120.*.*.*.music.tuple", "Same BPM (120)", 2},
        {"*.*.4-4.*.*.music.tuple", "Same time signature (4/4)", 3},
        {"*.*.*.c-major.*.music.tuple", "Same key (C major)", 2},
        {"*.*.*.major.*.music.tuple", "Same key family (major)", 3}
    };
    
    int num_collab = sizeof(collab_queries) / sizeof(collab_queries[0]);
    
    for (int i = 0; i < num_collab; i++) {
        printf("  %s (Priority: %d)\n", collab_queries[i].description, collab_queries[i].priority);
        printf("    Pattern: %s\n", collab_queries[i].pattern);
        
        int found = 0;
        for (int j = 0; j < num_nodes; j++) {
            /* Skip the source studio itself */
            if (strcmp(music_nodes[j].coordinate, source_coord) != 0 &&
                tupledns_match_pattern(music_nodes[j].coordinate, collab_queries[i].pattern)) {
                
                if (found == 0) {
                    printf("    Matches:\n");
                }
                printf("      - %s: %s\n", music_nodes[j].name, music_nodes[j].coordinate);
                found++;
            }
        }
        if (found == 0) {
            printf("    No matches found\n");
        }
        printf("\n");
    }
    
    /* Capability-based matching */
    printf("6. Capability-Based Matching\n");
    printf("----------------------------\n");
    
    const char* desired_capabilities[] = {
        "collaborative",
        "real-time",
        "midi",
        "generative"
    };
    
    int num_caps = sizeof(desired_capabilities) / sizeof(desired_capabilities[0]);
    
    for (int i = 0; i < num_caps; i++) {
        printf("  Looking for capability: %s\n", desired_capabilities[i]);
        
        int found = 0;
        for (int j = 0; j < num_nodes; j++) {
            /* Check if node has this capability */
            int has_capability = 0;
            for (int k = 0; music_nodes[j].capabilities[k] != NULL; k++) {
                if (strcmp(music_nodes[j].capabilities[k], desired_capabilities[i]) == 0) {
                    has_capability = 1;
                    break;
                }
            }
            
            if (has_capability) {
                if (found == 0) {
                    printf("    Available:\n");
                }
                printf("      - %s: %s\n", music_nodes[j].name, music_nodes[j].coordinate);
                found++;
            }
        }
        if (found == 0) {
            printf("    No matches found\n");
        }
        printf("\n");
    }
    
    /* Range-based BPM matching */
    printf("7. BPM Range Matching\n");
    printf("---------------------\n");
    printf("Finding collaborators within ±10 BPM of 120:\n\n");
    
    int target_bpm = 120;
    int bpm_tolerance = 10;
    int found = 0;
    
    for (int j = 0; j < num_nodes; j++) {
        char* space_type;
        char** values;
        int value_count;
        
        result = tupledns_decode_coordinate(music_nodes[j].coordinate, &space_type, &values, &value_count);
        if (result == TUPLEDNS_OK && value_count >= 2) {
            int bpm = atoi(values[1]);
            if (abs(bpm - target_bpm) <= bpm_tolerance) {
                if (found == 0) {
                    printf("  Compatible BPM range:\n");
                }
                printf("    - %s (%d BPM): %s\n", music_nodes[j].name, bpm, music_nodes[j].coordinate);
                found++;
            }
            
            free(space_type);
            tupledns_free_string_array(values, value_count);
        }
    }
    
    if (found == 0) {
        printf("  No matches in BPM range\n");
    }
    
    /* Test live DNS discovery */
    printf("\n8. Live DNS Discovery Test\n");
    printf("--------------------------\n");
    printf("Note: This would query actual DNS servers if configured.\n\n");
    
    tupledns_result_t* find_result = tupledns_find("*.120.*.*.*.music.tuple");
    if (find_result) {
        printf("  DNS Query Results:\n");
        printf("    Found nodes: %d\n", find_result->node_count);
        printf("    Query time: %.3f seconds\n", find_result->query_time);
        printf("    Status: %s\n", tupledns_error_string(find_result->error));
        
        tupledns_free_result(find_result);
    }
    
    /* Cleanup */
    tupledns_cleanup();
    
    printf("\nMusic collaboration example completed!\n");
    printf("This demonstrates how TupleDNS can organize musical artists\n");
    printf("and studios in a multidimensional coordinate space for\n");
    printf("intelligent collaboration matching.\n");
    
    return 0;
}