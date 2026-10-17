#include "rg_test.h"
#include "rg_wasm_bridge.h"
#include "rg_alloc.h"
#include <stdio.h>
#include <string.h>

int g_tests_total = 0;
int g_tests_passed = 0;
int g_tests_failed = 0;

void test_rg_x_11_wasm_bridge(void) {
    RG_TEST_START("RG-X-11");

    /* 1. Initialize bridge */
    int init_st = rg_wasm_init();
    RG_ASSERT_INT_EQ(init_st, 0);

    /* 2. Graph JSON inspection */
    const char *graph_json = rg_wasm_get_graph_json();
    RG_ASSERT(graph_json != NULL);
    RG_ASSERT(strstr(graph_json, "\"n_vertices\":12") != NULL);
    RG_ASSERT(strstr(graph_json, "\"n_edges\":13") != NULL);
    RG_ASSERT(strstr(graph_json, "Main Gate") != NULL);
    RG_ASSERT(strstr(graph_json, "Medical Annex") != NULL);

    /* 3. BFS baseline via List */
    const char *bfs_list_json = rg_wasm_run_bfs(0, 10, 0);
    RG_ASSERT(bfs_list_json != NULL);
    RG_ASSERT(strstr(bfs_list_json, "\"found\":true") != NULL);
    RG_ASSERT(strstr(bfs_list_json, "\"hop_count\":8") != NULL);
    RG_ASSERT(strstr(bfs_list_json, "\"neighbor_slots_examined\":26") != NULL);

    /* 4. BFS baseline via Matrix */
    const char *bfs_mat_json = rg_wasm_run_bfs(0, 10, 1);
    RG_ASSERT(bfs_mat_json != NULL);
    RG_ASSERT(strstr(bfs_mat_json, "\"found\":true") != NULL);
    RG_ASSERT(strstr(bfs_mat_json, "\"hop_count\":8") != NULL);
    RG_ASSERT(strstr(bfs_mat_json, "\"neighbor_slots_examined\":144") != NULL);

    /* 5. DFS baseline */
    const char *dfs_json = rg_wasm_run_dfs(0, 10, 0);
    RG_ASSERT(dfs_json != NULL);
    RG_ASSERT(strstr(dfs_json, "\"reachable\":true") != NULL);
    RG_ASSERT(strstr(dfs_json, "\"stale_pops\":2") != NULL);
    RG_ASSERT(strstr(dfs_json, "\"order_len\":12") != NULL);

    /* 6. Edge e9 cut (critical link) */
    int dis_e9 = rg_wasm_set_edge_active(9, 0);
    RG_ASSERT_INT_EQ(dis_e9, 0);

    const char *bfs_cut_json = rg_wasm_run_bfs(0, 10, 0);
    RG_ASSERT(strstr(bfs_cut_json, "\"found\":false") != NULL);
    RG_ASSERT(strstr(bfs_cut_json, "\"reachable\":false") != NULL);
    RG_ASSERT(strstr(bfs_cut_json, "\"hop_count\":-1") != NULL);

    const char *comp_cut_json = rg_wasm_get_components(0);
    RG_ASSERT(strstr(comp_cut_json, "\"component_count\":2") != NULL);

    /* 7. Re-enable e9 */
    int en_e9 = rg_wasm_set_edge_active(9, 1);
    RG_ASSERT_INT_EQ(en_e9, 0);

    const char *bfs_restored_json = rg_wasm_run_bfs(0, 10, 0);
    RG_ASSERT(strstr(bfs_restored_json, "\"found\":true") != NULL);
    RG_ASSERT(strstr(bfs_restored_json, "\"hop_count\":8") != NULL);

    /* 8. Edge e5 cut (redundant path) -> reroute via Block B */
    int dis_e5 = rg_wasm_set_edge_active(5, 0);
    RG_ASSERT_INT_EQ(dis_e5, 0);

    const char *bfs_e5_json = rg_wasm_run_bfs(0, 10, 0);
    RG_ASSERT(strstr(bfs_e5_json, "\"found\":true") != NULL);
    RG_ASSERT(strstr(bfs_e5_json, "\"hop_count\":8") != NULL);

    /* 9. Reset */
    int rst_st = rg_wasm_reset();
    RG_ASSERT_INT_EQ(rst_st, 0);

    /* 10. Failure table */
    const char *fail_tbl_json = rg_wasm_get_failure_table(0, 10);
    RG_ASSERT(fail_tbl_json != NULL);
    RG_ASSERT(strstr(fail_tbl_json, "\"n_rows\":13") != NULL);
    RG_ASSERT(strstr(fail_tbl_json, "\"edge_id\":9") != NULL);

    /* 11. SVG export */
    const char *svg_xml = rg_wasm_export_svg(0, 10, 0);
    RG_ASSERT(svg_xml != NULL);
    RG_ASSERT(strstr(svg_xml, "<svg") != NULL);
    RG_ASSERT(strstr(svg_xml, "SYNTHETIC NETWORK") != NULL);

    RG_TEST_END();
}

int main(void) {
    printf("========================================================\n");
    printf("RouteGuard WebAssembly Bridge Isolated Contract Test\n");
    printf("========================================================\n\n");

    test_rg_x_11_wasm_bridge();

    printf("\n========================================================\n");
    printf("BRIDGE TEST RESULTS: %d Total | %d Passed | %d Failed\n",
           g_tests_total, g_tests_passed, g_tests_failed);
    printf("========================================================\n");

    return (g_tests_failed == 0) ? 0 : 1;
}
