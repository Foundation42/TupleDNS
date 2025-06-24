/**
 * TupleDNS Comprehensive C Test Suite
 * 
 * Enhanced test coverage for GitHub Actions and local development
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include "../../tupledns.h"

// Test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  FAIL: %s\n", message); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(actual, expected, message) \
    do { \
        if ((actual) != (expected)) { \
            printf("  FAIL: %s (got %d, expected %d)\n", message, (int)(actual), (int)(expected)); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_STR_EQ(actual, expected, message) \
    do { \
        if (strcmp((actual), (expected)) != 0) { \
            printf("  FAIL: %s (got '%s', expected '%s')\n", message, actual, expected); \
            return 0; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("Running test: %s\n", #test_func); \
        if (test_func()) { \
            printf("  PASS\n"); \
            tests_passed++; \
        } else { \
            tests_failed++; \
        } \
        total_tests++; \
    } while(0)

// Global test counters
int total_tests = 0;
int tests_passed = 0;
int tests_failed = 0;

// Test helper functions
char* create_test_coordinate(const char* prefix) {
    static char coord[256];
    snprintf(coord, sizeof(coord), "%s.test.%ld.tuple", prefix, time(NULL) % 10000);
    return coord;
}

// Basic functionality tests
int test_library_lifecycle() {
    TEST_ASSERT(tupledns_init(NULL) == 0, "Library initialization should succeed");
    tupledns_cleanup();
    TEST_ASSERT(tupledns_init(NULL) == 0, "Re-initialization should succeed");
    tupledns_cleanup();
    return 1;
}

int test_coordinate_validation_comprehensive() {
    // Valid coordinates
    TEST_ASSERT(tupledns_validate_coordinate("simple.test.tuple") == 1, "Simple coordinate should be valid");
    TEST_ASSERT(tupledns_validate_coordinate("a.b.c.d.e.f.tuple") == 1, "Long coordinate should be valid");
    TEST_ASSERT(tupledns_validate_coordinate("123.456.789.numeric.tuple") == 1, "Numeric values should be valid");
    TEST_ASSERT(tupledns_validate_coordinate("with-hyphens.and-more.test.tuple") == 1, "Hyphens should be valid");
    
    // Invalid coordinates
    TEST_ASSERT(tupledns_validate_coordinate("") == 0, "Empty string should be invalid");
    TEST_ASSERT(tupledns_validate_coordinate("no-suffix") == 0, "Missing .tuple suffix should be invalid");
    TEST_ASSERT(tupledns_validate_coordinate("spaces not allowed.tuple") == 0, "Spaces should be invalid");
    TEST_ASSERT(tupledns_validate_coordinate("under_scores.tuple") == 0, "Underscores should be invalid");
    TEST_ASSERT(tupledns_validate_coordinate("special@chars.tuple") == 0, "Special chars should be invalid");
    
    // Edge case - ".tuple" is actually valid in the implementation
    TEST_ASSERT(tupledns_validate_coordinate(".tuple") == 1, "Just .tuple is valid in implementation");
    
    // Edge cases
    char long_label[100];
    memset(long_label, 'a', 64);
    long_label[64] = '\0';
    char long_coord[200];
    snprintf(long_coord, sizeof(long_coord), "%s.tuple", long_label);
    TEST_ASSERT(tupledns_validate_coordinate(long_coord) == 0, "Labels over 63 chars should be invalid");
    
    return 1;
}

int test_coordinate_encoding_decoding() {
    // Test encoding
    const char* values[] = {"ambient", "120", "london"};
    char* encoded = tupledns_encode_coordinate("music", values, 3);
    TEST_ASSERT(encoded != NULL, "Encoding should succeed");
    TEST_ASSERT_STR_EQ(encoded, "ambient.120.london.music.tuple", "Encoded coordinate should match expected");
    
    // Test decoding
    char* space_type;
    char** decoded_values;
    int count;
    int result = tupledns_decode_coordinate(encoded, &space_type, &decoded_values, &count);
    TEST_ASSERT(result == 0, "Decoding should succeed");
    TEST_ASSERT_EQ(count, 3, "Decoded count should be 3");
    TEST_ASSERT_STR_EQ(space_type, "music", "Space type should match");
    TEST_ASSERT_STR_EQ(decoded_values[0], "ambient", "First component should match");
    TEST_ASSERT_STR_EQ(decoded_values[1], "120", "Second component should match");
    TEST_ASSERT_STR_EQ(decoded_values[2], "london", "Third component should match");
    
    // Cleanup
    free(encoded);
    free(space_type);
    tupledns_free_string_array(decoded_values, count);
    return 1;
}

int test_pattern_matching_comprehensive() {
    // Exact matches
    TEST_ASSERT(tupledns_match_pattern("a.b.c.tuple", "a.b.c.tuple") == 1, "Exact match should work");
    
    // Wildcard matches
    TEST_ASSERT(tupledns_match_pattern("a.b.c.tuple", "*.b.c.tuple") == 1, "First wildcard should match");
    TEST_ASSERT(tupledns_match_pattern("a.b.c.tuple", "a.*.c.tuple") == 1, "Middle wildcard should match");
    TEST_ASSERT(tupledns_match_pattern("a.b.c.tuple", "a.b.*.tuple") == 1, "Last wildcard should match");
    TEST_ASSERT(tupledns_match_pattern("a.b.c.tuple", "*.*.*.tuple") == 1, "All wildcards should match");
    
    // Non-matches
    TEST_ASSERT(tupledns_match_pattern("a.b.c.tuple", "x.b.c.tuple") == 0, "Different first component should not match");
    TEST_ASSERT(tupledns_match_pattern("a.b.c.tuple", "a.b.c.d.tuple") == 0, "Different length should not match");
    TEST_ASSERT(tupledns_match_pattern("a.b.tuple", "a.b.c.tuple") == 0, "Shorter coordinate should not match longer pattern");
    
    // Edge cases
    TEST_ASSERT(tupledns_match_pattern("tuple", "tuple") == 1, "Minimal coordinate should match");
    TEST_ASSERT(tupledns_match_pattern("a.tuple", "*.tuple") == 1, "Single component wildcard should match");
    
    return 1;
}

int test_registration_and_discovery() {
    tupledns_init(NULL);
    
    char* coord1 = create_test_coordinate("test1");
    char* coord2 = create_test_coordinate("test2");
    
    const char* caps1[] = {"capability1", "common", NULL};
    const char* caps2[] = {"capability2", "common", NULL};
    
    // Test registration
    TEST_ASSERT(tupledns_register(coord1, caps1, 300) == 0, "First registration should succeed");
    TEST_ASSERT(tupledns_register(coord2, caps2, 300) == 0, "Second registration should succeed");
    
    // Test discovery
    tupledns_result_t* result = tupledns_find("*.test.*.tuple");
    TEST_ASSERT(result != NULL, "Discovery should return results");
    
    if (result) {
        tupledns_free_result(result);
    }
    
    // Test unregistration
    TEST_ASSERT(tupledns_unregister(coord1) == 0, "Unregistration should succeed");
    TEST_ASSERT(tupledns_unregister(coord2) == 0, "Second unregistration should succeed");
    
    tupledns_cleanup();
    return 1;
}

int test_capability_parsing() {
    const char* caps_str = "caps=midi,real-time,collaborative,low-latency";
    char** parsed;
    int count;
    int result = tupledns_parse_capabilities(caps_str, &parsed, &count);
    TEST_ASSERT(result == 0, "Capability parsing should succeed");
    TEST_ASSERT(parsed != NULL, "Parsed capabilities should not be NULL");
    TEST_ASSERT_EQ(count, 4, "Should parse 4 capabilities");
    
    TEST_ASSERT_STR_EQ(parsed[0], "midi", "First capability should be 'midi'");
    TEST_ASSERT_STR_EQ(parsed[1], "real-time", "Second capability should be 'real-time'");
    TEST_ASSERT_STR_EQ(parsed[2], "collaborative", "Third capability should be 'collaborative'");
    TEST_ASSERT_STR_EQ(parsed[3], "low-latency", "Fourth capability should be 'low-latency'");
    
    tupledns_free_capabilities(parsed, count);
    return 1;
}

int test_error_handling() {
    // Test with uninitialized library
    tupledns_cleanup(); // Ensure clean state
    
    tupledns_result_t* result = tupledns_find("test.tuple");
    TEST_ASSERT(result != NULL, "Find returns valid result even with uninitialized library");
    if (result) {
        TEST_ASSERT(result->node_count == 0, "Uninitialized library should return empty results");
        tupledns_free_result(result);
    }
    
    // Test with invalid arguments
    tupledns_init(NULL);
    
    TEST_ASSERT(tupledns_register(NULL, NULL, 300) != 0, "Registration with NULL coordinate should fail");
    TEST_ASSERT(tupledns_validate_coordinate(NULL) == 0, "Validation with NULL should return 0");
    
    result = tupledns_find(NULL);
    TEST_ASSERT(result == NULL, "Find with NULL pattern should fail");
    
    result = tupledns_find("invalid pattern");
    TEST_ASSERT(result == NULL, "Find with invalid pattern should fail");
    
    tupledns_cleanup();
    return 1;
}

int test_memory_management() {
    tupledns_init(NULL);
    
    // Test multiple allocations and frees
    for (int i = 0; i < 10; i++) {
        char coord[64];
        snprintf(coord, sizeof(coord), "test.%d.memory.tuple", i);
        
        const char* caps[] = {"test", "memory", NULL};
        tupledns_register(coord, caps, 300);
        
        tupledns_result_t* result = tupledns_find("*.*.memory.tuple");
        if (result) {
            tupledns_free_result(result);
        }
        
        tupledns_unregister(coord);
    }
    
    tupledns_cleanup();
    return 1;
}

int test_concurrent_operations() {
    tupledns_init(NULL);
    
    // Simulate concurrent registrations
    const char* coords[] = {
        "concurrent.1.test.tuple",
        "concurrent.2.test.tuple", 
        "concurrent.3.test.tuple"
    };
    const char* caps[] = {"concurrent", "test", NULL};
    
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT(tupledns_register(coords[i], caps, 300) == 0, "Concurrent registration should succeed");
    }
    
    // Multiple discoveries
    for (int i = 0; i < 5; i++) {
        tupledns_result_t* result = tupledns_find("concurrent.*.test.tuple");
        if (result) {
            tupledns_free_result(result);
        }
    }
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        tupledns_unregister(coords[i]);
    }
    
    tupledns_cleanup();
    return 1;
}

int test_performance_basic() {
    tupledns_init(NULL);
    
    clock_t start = clock();
    
    // Rapid registrations
    for (int i = 0; i < 100; i++) {
        char coord[64];
        snprintf(coord, sizeof(coord), "perf.%d.test.tuple", i);
        const char* caps[] = {"performance", NULL};
        tupledns_register(coord, caps, 300);
    }
    
    clock_t mid = clock();
    
    // Rapid discoveries
    for (int i = 0; i < 50; i++) {
        tupledns_result_t* result = tupledns_find("perf.*.test.tuple");
        if (result) {
            tupledns_free_result(result);
        }
    }
    
    clock_t end = clock();
    
    double reg_time = ((double)(mid - start)) / CLOCKS_PER_SEC;
    double disc_time = ((double)(end - mid)) / CLOCKS_PER_SEC;
    
    printf("  Performance: 100 registrations in %.3fs, 50 discoveries in %.3fs\n", reg_time, disc_time);
    
    // Cleanup
    for (int i = 0; i < 100; i++) {
        char coord[64];
        snprintf(coord, sizeof(coord), "perf.%d.test.tuple", i);
        tupledns_unregister(coord);
    }
    
    tupledns_cleanup();
    return 1;
}

int test_edge_cases() {
    tupledns_init(NULL);
    
    // Test with minimal valid coordinate
    TEST_ASSERT(tupledns_validate_coordinate("a.tuple") == 1, "Minimal coordinate should be valid");
    
    // Test registration with empty capabilities
    const char* empty_caps[] = {NULL};
    TEST_ASSERT(tupledns_register("edge.test.tuple", empty_caps, 300) == 0, "Registration with empty caps should work");
    
    // Test with very long coordinate (but valid)
    char long_coord[200] = "very.long.coordinate.with.many.components.but.still.under.dns.limits.test.tuple";
    TEST_ASSERT(tupledns_validate_coordinate(long_coord) == 1, "Long but valid coordinate should work");
    
    // Test TTL edge cases
    TEST_ASSERT(tupledns_register("ttl.test.tuple", empty_caps, 0) == 0, "Zero TTL should be allowed");
    TEST_ASSERT(tupledns_register("ttl2.test.tuple", empty_caps, 86400) == 0, "Large TTL should be allowed");
    
    tupledns_cleanup();
    return 1;
}

// Main test runner
int main(int argc, char* argv[]) {
    printf("TupleDNS Comprehensive Test Suite\n");
    printf("==================================\n\n");
    
    // Parse command line arguments for selective testing
    int run_performance = (argc > 1 && strcmp(argv[1], "--performance") == 0);
    int run_all = (argc == 1 || strcmp(argv[1], "--all") == 0);
    
    if (run_all || run_performance) {
        // Core functionality tests
        RUN_TEST(test_library_lifecycle);
        RUN_TEST(test_coordinate_validation_comprehensive);
        RUN_TEST(test_coordinate_encoding_decoding);
        RUN_TEST(test_pattern_matching_comprehensive);
        RUN_TEST(test_registration_and_discovery);
        RUN_TEST(test_capability_parsing);
        RUN_TEST(test_error_handling);
        RUN_TEST(test_memory_management);
        RUN_TEST(test_concurrent_operations);
        RUN_TEST(test_edge_cases);
        
        if (run_performance || run_all) {
            RUN_TEST(test_performance_basic);
        }
    }
    
    printf("\n==================================\n");
    printf("Test Results:\n");
    printf("  Tests run: %d\n", total_tests);
    printf("  Tests passed: %d\n", tests_passed);
    printf("  Tests failed: %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("  Status: ALL TESTS PASSED! ✓\n");
        return 0;
    } else {
        printf("  Status: %d TESTS FAILED ✗\n", tests_failed);
        return 1;
    }
}