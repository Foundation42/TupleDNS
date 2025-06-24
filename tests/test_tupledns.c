/**
 * TupleDNS Test Suite
 * 
 * Comprehensive tests for the TupleDNS library
 */

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include "tupledns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

/* Test utilities */
#define TEST(name) \
    do { \
        printf("Running test: %s\n", name); \
        tests_run++; \
    } while(0)

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  FAIL: %s\n", message); \
            return 0; \
        } \
    } while(0)

#define ASSERT_EQ(a, b, message) \
    do { \
        if ((a) != (b)) { \
            printf("  FAIL: %s (expected %d, got %d)\n", message, (int)(b), (int)(a)); \
            return 0; \
        } \
    } while(0)

#define ASSERT_STR_EQ(a, b, message) \
    do { \
        if (strcmp((a), (b)) != 0) { \
            printf("  FAIL: %s (expected '%s', got '%s')\n", message, (b), (a)); \
            return 0; \
        } \
    } while(0)

#define PASS() \
    do { \
        printf("  PASS\n"); \
        tests_passed++; \
        return 1; \
    } while(0)

/* Test Functions */

int test_initialization() {
    TEST("Initialization");
    
    /* Test default config */
    tupledns_config_t config = tupledns_default_config();
    ASSERT(config.timeout == TUPLEDNS_DEFAULT_TIMEOUT, "Default timeout incorrect");
    ASSERT(config.max_concurrent == 16, "Default max_concurrent incorrect");
    ASSERT(config.enable_caching == 1, "Default caching should be enabled");
    
    /* Test initialization */
    int result = tupledns_init(&config);
    ASSERT_EQ(result, TUPLEDNS_OK, "Initialization should succeed");
    
    /* Test double initialization */
    result = tupledns_init(&config);
    ASSERT_EQ(result, TUPLEDNS_OK, "Double initialization should succeed");
    
    /* Test cleanup */
    tupledns_cleanup();
    
    PASS();
}

int test_coordinate_validation() {
    TEST("Coordinate Validation");
    
    tupledns_init(NULL);
    
    /* Valid coordinates */
    ASSERT(tupledns_validate_coordinate("ambient.120.experimental.music.tuple"), "Valid music coordinate");
    ASSERT(tupledns_validate_coordinate("london.uk.europe.spatial.tuple"), "Valid spatial coordinate");
    ASSERT(tupledns_validate_coordinate("14.30.24.06.2025.time.tuple"), "Valid time coordinate");
    ASSERT(tupledns_validate_coordinate("test.tuple"), "Minimal valid coordinate");
    
    /* Invalid coordinates */
    ASSERT(!tupledns_validate_coordinate(NULL), "NULL coordinate should be invalid");
    ASSERT(!tupledns_validate_coordinate(""), "Empty coordinate should be invalid");
    ASSERT(!tupledns_validate_coordinate("no-tuple-suffix"), "Missing .tuple suffix");
    ASSERT(!tupledns_validate_coordinate("has spaces.tuple"), "Spaces should be invalid");
    ASSERT(!tupledns_validate_coordinate("has@symbol.tuple"), "Invalid characters");
    
    /* Test very long coordinate */
    char long_coord[300];
    memset(long_coord, 'a', 299);
    strcpy(long_coord + 299 - 6, ".tuple");
    ASSERT(!tupledns_validate_coordinate(long_coord), "Overly long coordinate should be invalid");
    
    tupledns_cleanup();
    PASS();
}

int test_coordinate_encoding() {
    TEST("Coordinate Encoding");
    
    tupledns_init(NULL);
    
    /* Test encoding */
    const char* values[] = {"ambient", "120", "experimental"};
    char* encoded = tupledns_encode_coordinate("music", values, 3);
    ASSERT(encoded != NULL, "Encoding should succeed");
    ASSERT_STR_EQ(encoded, "ambient.120.experimental.music.tuple", "Encoded coordinate format");
    
    /* Test decoding */
    char* space_type;
    char** decoded_values;
    int value_count;
    
    int result = tupledns_decode_coordinate(encoded, &space_type, &decoded_values, &value_count);
    ASSERT_EQ(result, TUPLEDNS_OK, "Decoding should succeed");
    ASSERT_STR_EQ(space_type, "music", "Decoded space type");
    ASSERT_EQ(value_count, 3, "Decoded value count");
    ASSERT_STR_EQ(decoded_values[0], "ambient", "First decoded value");
    ASSERT_STR_EQ(decoded_values[1], "120", "Second decoded value");
    ASSERT_STR_EQ(decoded_values[2], "experimental", "Third decoded value");
    
    /* Cleanup */
    free(encoded);
    free(space_type);
    tupledns_free_string_array(decoded_values, value_count);
    
    /* Test error cases */
    encoded = tupledns_encode_coordinate(NULL, values, 3);
    ASSERT(encoded == NULL, "NULL space_type should fail");
    
    encoded = tupledns_encode_coordinate("music", NULL, 3);
    ASSERT(encoded == NULL, "NULL values should fail");
    
    encoded = tupledns_encode_coordinate("music", values, 0);
    ASSERT(encoded == NULL, "Zero value count should fail");
    
    tupledns_cleanup();
    PASS();
}

int test_pattern_matching() {
    TEST("Pattern Matching");
    
    tupledns_init(NULL);
    
    const char* coordinate = "ambient.120.experimental.music.tuple";
    
    /* Exact match */
    ASSERT(tupledns_match_pattern(coordinate, "ambient.120.experimental.music.tuple"), "Exact match");
    
    /* Wildcard matches */
    ASSERT(tupledns_match_pattern(coordinate, "*.120.experimental.music.tuple"), "Wildcard genre");
    ASSERT(tupledns_match_pattern(coordinate, "ambient.*.experimental.music.tuple"), "Wildcard BPM");
    ASSERT(tupledns_match_pattern(coordinate, "ambient.120.*.music.tuple"), "Wildcard style");
    ASSERT(tupledns_match_pattern(coordinate, "*.*.*.music.tuple"), "Multiple wildcards");
    ASSERT(tupledns_match_pattern(coordinate, "*.*.*.*.*"), "All wildcards");
    
    /* Non-matches */
    ASSERT(!tupledns_match_pattern(coordinate, "jazz.120.experimental.music.tuple"), "Different genre");
    ASSERT(!tupledns_match_pattern(coordinate, "ambient.140.experimental.music.tuple"), "Different BPM");
    ASSERT(!tupledns_match_pattern(coordinate, "ambient.120.experimental.spatial.tuple"), "Different space");
    ASSERT(!tupledns_match_pattern(coordinate, "ambient.120.experimental.music"), "Missing .tuple");
    
    /* Different lengths */
    ASSERT(!tupledns_match_pattern(coordinate, "ambient.120.music.tuple"), "Fewer components");
    ASSERT(!tupledns_match_pattern(coordinate, "ambient.120.experimental.jazz.music.tuple"), "More components");
    
    /* Edge cases */
    ASSERT(!tupledns_match_pattern(NULL, "*.tuple"), "NULL coordinate");
    ASSERT(!tupledns_match_pattern("test.tuple", NULL), "NULL pattern");
    ASSERT(!tupledns_match_pattern(NULL, NULL), "Both NULL");
    
    tupledns_cleanup();
    PASS();
}

int test_string_utilities() {
    TEST("String Utilities");
    
    /* Test string splitting */
    int count;
    char** parts = tupledns_split_string("a.b.c.d", ".", &count);
    ASSERT(parts != NULL, "Split should succeed");
    ASSERT_EQ(count, 4, "Split count");
    ASSERT_STR_EQ(parts[0], "a", "First part");
    ASSERT_STR_EQ(parts[1], "b", "Second part");
    ASSERT_STR_EQ(parts[2], "c", "Third part");
    ASSERT_STR_EQ(parts[3], "d", "Fourth part");
    tupledns_free_string_array(parts, count);
    
    /* Test string joining */
    const char* strings[] = {"a", "b", "c", "d"};
    char* joined = tupledns_join_strings(strings, 4, ".");
    ASSERT(joined != NULL, "Join should succeed");
    ASSERT_STR_EQ(joined, "a.b.c.d", "Joined string");
    free(joined);
    
    /* Test edge cases */
    parts = tupledns_split_string("", ".", &count);
    ASSERT(parts == NULL, "Empty string split should return NULL");
    
    parts = tupledns_split_string("no-separator", ".", &count);
    ASSERT(parts != NULL, "Single token should work");
    ASSERT_EQ(count, 1, "Single token count");
    ASSERT_STR_EQ(parts[0], "no-separator", "Single token value");
    tupledns_free_string_array(parts, count);
    
    joined = tupledns_join_strings(NULL, 0, ".");
    ASSERT(joined == NULL, "NULL strings should return NULL");
    
    PASS();
}

int test_capability_parsing() {
    TEST("Capability Parsing");
    
    tupledns_init(NULL);
    
    /* Test basic capability parsing */
    const char* txt_record = "caps=midi-in,midi-out,real-time version=1.0";
    char** capabilities;
    int cap_count;
    
    int result = tupledns_parse_capabilities(txt_record, &capabilities, &cap_count);
    ASSERT_EQ(result, TUPLEDNS_OK, "Capability parsing should succeed");
    ASSERT_EQ(cap_count, 3, "Capability count");
    ASSERT_STR_EQ(capabilities[0], "midi-in", "First capability");
    ASSERT_STR_EQ(capabilities[1], "midi-out", "Second capability");
    ASSERT_STR_EQ(capabilities[2], "real-time", "Third capability");
    
    tupledns_free_capabilities(capabilities, cap_count);
    
    /* Test no capabilities */
    result = tupledns_parse_capabilities("version=1.0 other=value", &capabilities, &cap_count);
    ASSERT_EQ(result, TUPLEDNS_OK, "No caps should succeed");
    ASSERT_EQ(cap_count, 0, "No capabilities count");
    ASSERT(capabilities == NULL, "No capabilities array");
    
    /* Test capability copying */
    const char* orig_caps[] = {"midi", "real-time", "generative", NULL};
    char** copied = tupledns_copy_capabilities(orig_caps, 3);
    ASSERT(copied != NULL, "Capability copying should succeed");
    ASSERT_STR_EQ(copied[0], "midi", "First copied capability");
    ASSERT_STR_EQ(copied[1], "real-time", "Second copied capability");
    ASSERT_STR_EQ(copied[2], "generative", "Third copied capability");
    
    tupledns_free_capabilities(copied, 3);
    
    tupledns_cleanup();
    PASS();
}

int test_node_operations() {
    TEST("Node Operations");
    
    tupledns_init(NULL);
    
    /* Create a test node */
    tupledns_node_t node = {0};
    node.coordinate = strdup("test.120.music.tuple");
    node.ip_address = strdup("192.168.1.100");
    node.capability_count = 2;
    node.capabilities = malloc(2 * sizeof(char*));
    node.capabilities[0] = strdup("midi");
    node.capabilities[1] = strdup("real-time");
    node.ttl = 300;
    node.last_seen = 1234567890;
    
    /* Test capability checking */
    ASSERT(tupledns_has_capability(&node, "midi"), "Should have midi capability");
    ASSERT(tupledns_has_capability(&node, "real-time"), "Should have real-time capability");
    ASSERT(!tupledns_has_capability(&node, "nonexistent"), "Should not have nonexistent capability");
    
    /* Test node cleanup */
    tupledns_free_node(&node);
    ASSERT(node.coordinate == NULL, "Coordinate should be NULL after cleanup");
    ASSERT(node.ip_address == NULL, "IP address should be NULL after cleanup");
    ASSERT(node.capabilities == NULL, "Capabilities should be NULL after cleanup");
    ASSERT(node.capability_count == 0, "Capability count should be 0 after cleanup");
    
    tupledns_cleanup();
    PASS();
}

int test_error_handling() {
    TEST("Error Handling");
    
    tupledns_init(NULL);
    
    /* Test error string conversion */
    ASSERT_STR_EQ(tupledns_error_string(TUPLEDNS_OK), "Success", "Success error string");
    ASSERT_STR_EQ(tupledns_error_string(TUPLEDNS_ERROR_INVALID_COORDINATE), "Invalid coordinate format", "Invalid coordinate error string");
    ASSERT_STR_EQ(tupledns_error_string(TUPLEDNS_ERROR_DNS_QUERY_FAILED), "DNS query failed", "DNS query failed error string");
    
    /* Test unknown error */
    const char* unknown = tupledns_error_string(9999);
    ASSERT_STR_EQ(unknown, "Unknown error", "Unknown error string");
    
    tupledns_cleanup();
    PASS();
}

int test_discovery_api() {
    TEST("Discovery API");
    
    tupledns_init(NULL);
    
    /* Test basic discovery (will return empty result) */
    tupledns_result_t* result = tupledns_find("*.music.tuple");
    ASSERT(result != NULL, "Find should return result object");
    ASSERT_EQ(result->node_count, 0, "Should find no nodes without DNS server");
    ASSERT_EQ(result->total_queries, 1, "Should perform one query");
    ASSERT(result->query_time >= 0, "Query time should be non-negative");
    
    tupledns_free_result(result);
    
    /* Test NULL pattern */
    result = tupledns_find(NULL);
    ASSERT(result == NULL, "NULL pattern should return NULL");
    
    /* Test capability-based discovery */
    const char* caps[] = {"midi", NULL};
    result = tupledns_find_with_caps("*.music.tuple", caps);
    ASSERT(result != NULL, "Capability-based find should return result");
    tupledns_free_result(result);
    
    /* Test range query */
    tupledns_range_t ranges[] = {
        {"bpm", 100, 140}
    };
    result = tupledns_find_range("*.{bpm}.music.tuple", ranges, 1);
    ASSERT(result != NULL, "Range query should return result");
    tupledns_free_result(result);
    
    /* Test multi-pattern search */
    const char* patterns[] = {"*.music.tuple", "*.spatial.tuple"};
    result = tupledns_search_multi(patterns, 2);
    ASSERT(result != NULL, "Multi-pattern search should return result");
    tupledns_free_result(result);
    
    tupledns_cleanup();
    PASS();
}

int test_registration_api() {
    TEST("Registration API");
    
    tupledns_init(NULL);
    
    /* Test basic registration */
    const char* caps[] = {"midi", "real-time", NULL};
    int result = tupledns_register("test.120.music.tuple", caps, 300);
    ASSERT_EQ(result, TUPLEDNS_OK, "Registration should succeed (mock)");
    
    /* Test registration with IP */
    result = tupledns_register_with_ip("test.120.music.tuple", "192.168.1.100", caps, 300);
    ASSERT_EQ(result, TUPLEDNS_OK, "Registration with IP should succeed (mock)");
    
    /* Test unregistration */
    result = tupledns_unregister("test.120.music.tuple");
    ASSERT_EQ(result, TUPLEDNS_OK, "Unregistration should succeed (mock)");
    
    /* Test invalid coordinates */
    result = tupledns_register("invalid-coordinate", caps, 300);
    ASSERT_EQ(result, TUPLEDNS_ERROR_INVALID_COORDINATE, "Invalid coordinate should fail");
    
    result = tupledns_register_with_ip("invalid-coordinate", "192.168.1.100", caps, 300);
    ASSERT_EQ(result, TUPLEDNS_ERROR_INVALID_COORDINATE, "Invalid coordinate with IP should fail");
    
    result = tupledns_unregister("invalid-coordinate");
    ASSERT_EQ(result, TUPLEDNS_ERROR_INVALID_COORDINATE, "Invalid coordinate unregistration should fail");
    
    tupledns_cleanup();
    PASS();
}

/* Main test runner */
int main() {
    printf("TupleDNS Test Suite\n");
    printf("===================\n\n");
    
    /* Run all tests */
    test_initialization();
    test_coordinate_validation();
    test_coordinate_encoding();
    test_pattern_matching();
    test_string_utilities();
    test_capability_parsing();
    test_node_operations();
    test_error_handling();
    test_discovery_api();
    test_registration_api();
    
    /* Print results */
    printf("\n===================\n");
    printf("Test Results:\n");
    printf("  Tests run: %d\n", tests_run);
    printf("  Tests passed: %d\n", tests_passed);
    printf("  Tests failed: %d\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("  Status: ALL TESTS PASSED! ✓\n");
        return 0;
    } else {
        printf("  Status: SOME TESTS FAILED! ✗\n");
        return 1;
    }
}