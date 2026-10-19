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

    /* 12. Fixture switching: Switch to MUJ Campus Demo */
    int camp_init = rg_wasm_init_fixture(1);
    RG_ASSERT_INT_EQ(camp_init, 0);
    RG_ASSERT_INT_EQ(rg_wasm_get_active_fixture(), 1);

    const char *camp_json = rg_wasm_get_graph_json();
    RG_ASSERT(camp_json != NULL);
    RG_ASSERT(strstr(camp_json, "\"fixture_id\":1") != NULL);
    RG_ASSERT(strstr(camp_json, "\"fixture_name\":\"MUJ Campus Demo\"") != NULL);
    RG_ASSERT(strstr(camp_json, "\"n_vertices\":14") != NULL);
    RG_ASSERT(strstr(camp_json, "\"n_edges\":21") != NULL);
    RG_ASSERT(strstr(camp_json, "\"Main Gate\"") != NULL);
    RG_ASSERT(strstr(camp_json, "\"Academic Block 3\"") != NULL);
    RG_ASSERT(strstr(camp_json, "\"Knowledge Centre\"") != NULL);
    RG_ASSERT(strstr(camp_json, "\"VIP Gate\"") != NULL);
    RG_ASSERT(strstr(camp_json, "VIP Gate — Southeastern campus arrival & executive entry") != NULL);

    /* Campus BFS Baseline List: 0 to 6 (Academic Block 3) */
    const char *camp_bfs_list = rg_wasm_run_bfs(0, 6, 0);
    RG_ASSERT(camp_bfs_list != NULL);
    RG_ASSERT(strstr(camp_bfs_list, "\"found\":true") != NULL);
    RG_ASSERT(strstr(camp_bfs_list, "\"hop_count\":2") != NULL);
    RG_ASSERT(strstr(camp_bfs_list, "\"neighbor_slots_examined\":42") != NULL);

    /* Campus BFS Baseline Matrix: 0 to 6 */
    const char *camp_bfs_mat = rg_wasm_run_bfs(0, 6, 1);
    RG_ASSERT(camp_bfs_mat != NULL);
    RG_ASSERT(strstr(camp_bfs_mat, "\"found\":true") != NULL);
    RG_ASSERT(strstr(camp_bfs_mat, "\"hop_count\":2") != NULL);
    RG_ASSERT(strstr(camp_bfs_mat, "\"neighbor_slots_examined\":196") != NULL);

    /* Campus edge e12 cut (critical link between LHC and Football Ground) */
    int camp_cut_e12 = rg_wasm_set_edge_active(12, 0);
    RG_ASSERT_INT_EQ(camp_cut_e12, 0);

    const char *camp_cut_bfs = rg_wasm_run_bfs(0, 9, 0);
    RG_ASSERT(strstr(camp_cut_bfs, "\"found\":false") != NULL);
    RG_ASSERT(strstr(camp_cut_bfs, "\"reachable\":false") != NULL);
    RG_ASSERT(strstr(camp_cut_bfs, "\"hop_count\":-1") != NULL);

    const char *camp_cut_comp = rg_wasm_get_components(0);
    RG_ASSERT(strstr(camp_cut_comp, "\"component_count\":2") != NULL);

    /* Reset campus fixture */
    int camp_rst = rg_wasm_reset();
    RG_ASSERT_INT_EQ(camp_rst, 0);

    const char *camp_restored_bfs = rg_wasm_run_bfs(0, 9, 0);
    RG_ASSERT(strstr(camp_restored_bfs, "\"found\":true") != NULL);
    RG_ASSERT(strstr(camp_restored_bfs, "\"hop_count\":4") != NULL);

    /* Campus failure table */
    const char *camp_fail_tbl = rg_wasm_get_failure_table(0, 9);
    RG_ASSERT(camp_fail_tbl != NULL);
    RG_ASSERT(strstr(camp_fail_tbl, "\"n_rows\":21") != NULL);

    /* 13. Switch back to F1 Reference Graph */
    int f1_reinit = rg_wasm_init_fixture(0);
    RG_ASSERT_INT_EQ(f1_reinit, 0);
    RG_ASSERT_INT_EQ(rg_wasm_get_active_fixture(), 0);

    const char *f1_json = rg_wasm_get_graph_json();
    RG_ASSERT(strstr(f1_json, "\"fixture_id\":0") != NULL);
    RG_ASSERT(strstr(f1_json, "\"n_vertices\":12") != NULL);
    RG_ASSERT(strstr(f1_json, "\"n_edges\":13") != NULL);

    const char *f1_bfs = rg_wasm_run_bfs(0, 10, 0);
    RG_ASSERT(strstr(f1_bfs, "\"found\":true") != NULL);
    RG_ASSERT(strstr(f1_bfs, "\"hop_count\":8") != NULL);

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
