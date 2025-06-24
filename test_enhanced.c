#include <stdio.h>
#include "tupledns.h"
int main() {
    tupledns_init(NULL);
    
    printf("Testing registration with capabilities and TTL:
");
    const char* caps[] = {"midi", "real-time", "collaborative", NULL};
    int result = tupledns_register("ambient.120.london.music.tuple", caps, 600);
    printf("Registration result: %d
", result);
    
    printf("
Testing pattern expansion:
");
    tupledns_result_t* find_result = tupledns_find("ambient.*.*.music.tuple");
    if (find_result) {
        printf("Query time: %.3fs
", find_result->query_time);
        printf("Total queries: %d
", find_result->total_queries);
        printf("Nodes found: %d
", find_result->node_count);
        tupledns_free_result(find_result);
    }
    
    tupledns_cleanup();
    return 0;
}
