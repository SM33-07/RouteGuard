#include "rg_test.h"
#include "rg_graph.h"
#include "rg_traverse.h"
#include "rg_analysis.h"
#include "rg_fixture.h"

void test_rg_comp_01(void) {
    RG_TEST_START("RG-COMP-01");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);

    int c_mat = rg_connected_components_matrix(g, NULL);
    int c_list = rg_connected_components_list(g, NULL);

    RG_ASSERT_INT_EQ(c_mat, 1);
    RG_ASSERT_INT_EQ(c_list, 1);

    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_fail_01(void) {
    RG_TEST_START("RG-FAIL-01");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    int src = 0, dst = 10;

    /* Disable only connecting edge e9 (6-7) */
    RgNetworkState before, after;
    rg_get_network_state(g, src, dst, false, &before);
    RG_ASSERT_INT_EQ(before.route_hops, 8);
    RG_ASSERT_INT_EQ(before.reachable_src_count, 12);
    RG_ASSERT_INT_EQ(before.component_count, 1);

    RgStatus st_dis = rg_disable_edge_by_id(g, 9);
    RG_ASSERT_INT_EQ(st_dis, RG_OK);

    rg_get_network_state(g, src, dst, false, &after);
    RG_ASSERT(!after.route_found);
    RG_ASSERT_INT_EQ(after.route_hops, -1);
    RG_ASSERT_INT_EQ(after.reachable_src_count, 8);
    RG_ASSERT_INT_EQ(after.component_count, 2);

    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_fail_02(void) {
    RG_TEST_START("RG-FAIL-02");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    int src = 0, dst = 10;

    /* Disable redundant edge e5 (3-4) */
    rg_disable_edge_by_id(g, 5);

    RgNetworkState state;
    rg_get_network_state(g, src, dst, false, &state);
    RG_ASSERT(state.route_found);
    RG_ASSERT_INT_EQ(state.route_hops, 8);
    RG_ASSERT_INT_EQ(state.reachable_src_count, 12);
    RG_ASSERT_INT_EQ(state.component_count, 1);

    /* Verify rerouted path passes through Block B (vertex 5) */
    RgRoute *rt = NULL;
    rg_route_create(g->n_vertices, &rt);
    rg_find_route(g, src, dst, false, rt, NULL);
    int expected_reroute[] = {0, 1, 3, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i <= 8; i++) {
        RG_ASSERT_INT_EQ(rt->path[i], expected_reroute[i]);
    }

    rg_route_free(rt);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_fail_03(void) {
    RG_TEST_START("RG-FAIL-03");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    int src = 0, dst = 10;

    /* Disable e9 and e2 without manually re-enabling them, then call reset */
    rg_disable_edge_by_id(g, 9);
    rg_disable_edge_by_id(g, 2);
    rg_graph_reset(g);

    RgNetworkState baseline;
    rg_get_network_state(g, src, dst, false, &baseline);
    RG_ASSERT(baseline.route_found);
    RG_ASSERT_INT_EQ(baseline.route_hops, 8);
    RG_ASSERT_INT_EQ(baseline.reachable_src_count, 12);
    RG_ASSERT_INT_EQ(baseline.component_count, 1);

    for (int i = 0; i < g->n_edges; i++) {
        RG_ASSERT(g->edges[i].active);
    }

    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_x_01(void) {
    RG_TEST_START("RG-X-01");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);

    /* Enabling already active edge returns RG_NOOP */
    RG_ASSERT_INT_EQ(rg_enable_edge_by_id(g, 0), RG_NOOP);

    /* Disabling active edge returns RG_OK */
    RG_ASSERT_INT_EQ(rg_disable_edge_by_id(g, 0), RG_OK);

    /* Disabling already disabled edge returns RG_NOOP */
    RG_ASSERT_INT_EQ(rg_disable_edge_by_id(g, 0), RG_NOOP);

    /* Re-enabling returns RG_OK */
    RG_ASSERT_INT_EQ(rg_enable_edge_by_id(g, 0), RG_OK);

    /* Endpoints version test */
    RG_ASSERT_INT_EQ(rg_disable_edge_by_endpoints(g, 0, 1), RG_OK);
    RG_ASSERT_INT_EQ(rg_disable_edge_by_endpoints(g, 0, 1), RG_NOOP);
    RG_ASSERT_INT_EQ(rg_enable_edge_by_endpoints(g, 0, 1), RG_OK);
    RG_ASSERT_INT_EQ(rg_enable_edge_by_endpoints(g, 0, 1), RG_NOOP);

    /* Non-existent edge returns RG_ERR_NOT_FOUND */
    RG_ASSERT_INT_EQ(rg_disable_edge_by_endpoints(g, 0, 10), RG_ERR_NOT_FOUND);

    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_x_02(void) {
    RG_TEST_START("RG-X-02");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    int src = 0, dst = 10;

    RgFailureTable *tbl = NULL;
    rg_failure_table_create(g->n_edges, &tbl);
    rg_compute_failure_table(g, src, dst, tbl);

    RG_ASSERT_INT_EQ(tbl->n_rows, 13);

    /* Edges e9, e10, e11, e12 must disconnect 0 and 10 */
    for (int e = 9; e <= 12; e++) {
        RG_ASSERT(!tbl->rows[e].reachable);
        RG_ASSERT_INT_EQ(tbl->rows[e].hop_count, -1);
        RG_ASSERT_INT_EQ(tbl->rows[e].component_count, 2);
    }

    /* Edge e2 (0-11, Parking) isolates vertex 11: component_count = 2, route still 8 hops */
    RG_ASSERT(tbl->rows[2].reachable);
    RG_ASSERT_INT_EQ(tbl->rows[2].hop_count, 8);
    RG_ASSERT_INT_EQ(tbl->rows[2].reachable_src_count, 11);
    RG_ASSERT_INT_EQ(tbl->rows[2].component_count, 2);

    /* All other edges (e0, e1, e3, e4, e5, e6, e7, e8) keep destination reachable in 8 hops with 1 component */
    int redundant_edges[] = {0, 1, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < 8; i++) {
        int e = redundant_edges[i];
        RG_ASSERT(tbl->rows[e].reachable);
        RG_ASSERT_INT_EQ(tbl->rows[e].hop_count, 8);
        RG_ASSERT_INT_EQ(tbl->rows[e].reachable_src_count, 12);
        RG_ASSERT_INT_EQ(tbl->rows[e].component_count, 1);
    }

    /* Verify graph immutability restoration: all edges active */
    for (int e = 0; e < g->n_edges; e++) {
        RG_ASSERT(g->edges[e].active);
    }

    rg_failure_table_free(tbl);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_x_07(void) {
    RG_TEST_START("RG-X-07");
    /* F2: 4 components */
    RgGraph *f2 = NULL;
    rg_fixture_f2_disconnected(&f2);
    RG_ASSERT_INT_EQ(rg_connected_components_matrix(f2, NULL), 4);
    RG_ASSERT_INT_EQ(rg_connected_components_list(f2, NULL), 4);
    rg_graph_free(f2);

    /* F3: single vertex, 1 component */
    RgGraph *f3 = NULL;
    rg_fixture_f3_single_vertex(&f3);
    RG_ASSERT_INT_EQ(rg_connected_components_matrix(f3, NULL), 1);
    RG_ASSERT_INT_EQ(rg_connected_components_list(f3, NULL), 1);
    rg_graph_free(f3);

    RG_TEST_END();
}
