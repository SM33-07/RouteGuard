#ifndef RG_TEST_H
#define RG_TEST_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

extern int g_tests_total;
extern int g_tests_passed;
extern int g_tests_failed;

#define RG_TEST_START(name) \
    printf("Running %-20s ... ", name); \
    fflush(stdout); \
    int _test_failures_before = g_tests_failed; \
    g_tests_total++;

#define RG_TEST_END() \
    if (g_tests_failed == _test_failures_before) { \
        printf("[ PASS ]\n"); \
        g_tests_passed++; \
    } else { \
        printf("[ FAIL ]\n"); \
    }

#define RG_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("\n  ASSERTION FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
            g_tests_failed++; \
            return; \
        } \
    } while (0)

#define RG_ASSERT_INT_EQ(actual, expected) \
    do { \
        int _act = (int)(actual); \
        int _exp = (int)(expected); \
        if (_act != _exp) { \
            printf("\n  ASSERTION FAILED: %s == %s (actual=%d, expected=%d) (%s:%d)\n", \
                   #actual, #expected, _act, _exp, __FILE__, __LINE__); \
            g_tests_failed++; \
            return; \
        } \
    } while (0)

#define RG_ASSERT_STR_EQ(actual, expected) \
    do { \
        const char *_act = (actual); \
        const char *_exp = (expected); \
        if (strcmp(_act, _exp) != 0) { \
            printf("\n  ASSERTION FAILED: strcmp(%s, %s) == 0 (actual=\"%s\", expected=\"%s\") (%s:%d)\n", \
                   #actual, #expected, _act, _exp, __FILE__, __LINE__); \
            g_tests_failed++; \
            return; \
        } \
    } while (0)

#endif /* RG_TEST_H */
