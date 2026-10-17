#include "rg_test.h"
#include <stdio.h>

int g_tests_total = 0;
int g_tests_passed = 0;
int g_tests_failed = 0;

/* Graph & Validation Tests */
void test_rg_gr_01(void);
void test_rg_gr_02(void);
void test_rg_err_01(void);
void test_rg_mem_01(void);
void test_rg_x_04(void);
void test_rg_x_05(void);
void test_rg_x_06(void);

/* Traversal & Routing Tests */
void test_rg_bfs_01(void);
void test_rg_bfs_02(void);
void test_rg_bfs_03(void);
void test_rg_bfs_04(void);
void test_rg_dfs_01(void);
void test_rg_dfs_02(void);
void test_rg_rep_01(void);
void test_rg_rep_02(void);
void test_rg_x_08(void);
void test_rg_x_09(void);

/* Resilience & Analysis Tests */
void test_rg_comp_01(void);
void test_rg_fail_01(void);
void test_rg_fail_02(void);
void test_rg_fail_03(void);
void test_rg_x_01(void);
void test_rg_x_02(void);
void test_rg_x_07(void);

/* Metrics & Fixture Reference Tests */
void test_rg_met_01(void);
void test_rg_x_03(void);
void test_rg_x_12_campus(void);
void test_rg_x_10_differential(void);

int main(void) {
    printf("========================================================\n");
    printf("RouteGuard Automated Test Suite (SRS v0.2 Traceable)\n");
    printf("ISO C11 Scope - CSC2101 Lecture 37 Academic Boundary\n");
    printf("========================================================\n\n");

    /* Graph Model & Lifecyle */
    test_rg_gr_01();
    test_rg_gr_02();
    test_rg_err_01();
    test_rg_mem_01();
    test_rg_x_04();
    test_rg_x_05();
    test_rg_x_06();

    /* Traversal & Routes */
    test_rg_bfs_01();
    test_rg_bfs_02();
    test_rg_bfs_03();
    test_rg_bfs_04();
    test_rg_dfs_01();
    test_rg_dfs_02();
    test_rg_rep_01();
    test_rg_rep_02();
    test_rg_x_08();
    test_rg_x_09();

    /* Components & Failure Analysis */
    test_rg_comp_01();
    test_rg_fail_01();
    test_rg_fail_02();
    test_rg_fail_03();
    test_rg_x_01();
    test_rg_x_02();
    test_rg_x_07();

    /* Metrics & Reference Counters */
    test_rg_met_01();
    test_rg_x_03();
    test_rg_x_12_campus();

    /* Differential Testing against Independent Oracle */
    test_rg_x_10_differential();


    printf("\n========================================================\n");
    printf("TEST RESULTS: %d Total | %d Passed | %d Failed\n",
           g_tests_total, g_tests_passed, g_tests_failed);
    printf("========================================================\n");

    return (g_tests_failed == 0) ? 0 : 1;
}
