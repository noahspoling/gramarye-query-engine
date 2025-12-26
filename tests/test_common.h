#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <stdbool.h>
#include "assert.h"
#include "except.h"

// Test exception type
extern const Except_T Test_Failed;

// Test assertion macros using libcore's assert
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("    ✗ FAILED: %s\n", message); \
            RAISE(Test_Failed); \
        } \
    } while (0)

#define TEST_ASSERT_EQ(a, b, message) \
    do { \
        if ((a) != (b)) { \
            printf("    ✗ FAILED: %s (expected %d, got %d)\n", message, (int)(b), (int)(a)); \
            RAISE(Test_Failed); \
        } \
    } while (0)

#define TEST_ASSERT_NE(a, b, message) \
    do { \
        if ((a) == (b)) { \
            printf("    ✗ FAILED: %s\n", message); \
            RAISE(Test_Failed); \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr, message) \
    do { \
        if ((ptr) == NULL) { \
            printf("    ✗ FAILED: %s (pointer is NULL)\n", message); \
            RAISE(Test_Failed); \
        } \
    } while (0)

#define TEST_ASSERT_NULL(ptr, message) \
    do { \
        if ((ptr) != NULL) { \
            printf("    ✗ FAILED: %s (pointer is not NULL)\n", message); \
            RAISE(Test_Failed); \
        } \
    } while (0)

#define TEST_ASSERT_TRUE(condition, message) \
    do { \
        if (!(condition)) { \
            printf("    ✗ FAILED: %s (expected true)\n", message); \
            RAISE(Test_Failed); \
        } \
    } while (0)

#define TEST_ASSERT_FALSE(condition, message) \
    do { \
        if (condition) { \
            printf("    ✗ FAILED: %s (expected false)\n", message); \
            RAISE(Test_Failed); \
        } \
    } while (0)

// Helper macro to run a test function and catch exceptions
// Usage: if (!run_test_safe(test_func)) return false;

#endif // TEST_COMMON_H

