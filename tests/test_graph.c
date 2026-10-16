#include "rg_test.h"
#include "rg_graph.h"
#include "rg_fixture.h"
#include "rg_alloc.h"
#include "rg_traverse.h"

void test_rg_gr_01(void) {
    RG_TEST_START("RG-GR-01");
    RgGraph *g = NULL;
    RgStatus st = rg_fixture_f1_campus(&g);
    RG_ASSERT_INT_EQ(st, RG_OK);
    RG_ASSERT(g != NULL);
    RG_ASSERT_INT_EQ(g->n_vertices, 12);
    RG_ASSERT_INT_EQ(g->n_edges, 13);

    /* Verify matrix and list agreement on every vertex pair */
    for (int u = 0; u < g->n_vertices; u++) {
        for (int v = 0; v < g->n_vertices; v++) {
            int mat_eid = g->matrix[u * g->n_vertices + v];
            bool list_has_edge = false;
            int list_eid = -1;
            RgAdjNode *node = g->adj_list[u];
            while (node != NULL) {
                if (node->neighbor == v) {
                    list_has_edge = true;
                    list_eid = node->edge_id;
                    break;
                }
                node = node->next;
            }
            if (mat_eid == -1) {
                RG_ASSERT(!list_has_edge);
            } else {
                RG_ASSERT(list_has_edge);
                RG_ASSERT_INT_EQ(mat_eid, list_eid);
            }
        }
    }
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_gr_02(void) {
    RG_TEST_START("RG-GR-02");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    int prev_edges = g->n_edges;

    /* Out of bounds vertex rejection */
    int out_id = -1;
    RgStatus st1 = rg_add_edge(g, 0, 99, &out_id);
    RG_ASSERT_INT_EQ(st1, RG_ERR_INVALID_PARAM);
    RG_ASSERT_INT_EQ(g->n_edges, prev_edges);

    RgStatus st2 = rg_add_edge(g, -1, 3, &out_id);
    RG_ASSERT_INT_EQ(st2, RG_ERR_INVALID_PARAM);
    RG_ASSERT_INT_EQ(g->n_edges, prev_edges);

    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_err_01(void) {
    RG_TEST_START("RG-ERR-01");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    RgRoute *rt = NULL;
    rg_route_create(g->n_vertices, &rt);

    /* Invalid source or destination */
    RgStatus st1 = rg_find_route(g, -1, 5, false, rt, NULL);
    RG_ASSERT_INT_EQ(st1, RG_ERR_INVALID_PARAM);

    RgStatus st2 = rg_find_route(g, 0, 99, false, rt, NULL);
    RG_ASSERT_INT_EQ(st2, RG_ERR_INVALID_PARAM);

    RgStatus st3 = rg_find_route(NULL, 0, 5, false, rt, NULL);
    RG_ASSERT_INT_EQ(st3, RG_ERR_INVALID_PARAM);

    rg_route_free(rt);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_mem_01(void) {
    RG_TEST_START("RG-MEM-01");
    size_t base_allocs = rg_alloc_active_count();

    /* Repeated create/reset/free loop */
    for (int iter = 0; iter < 10; iter++) {
        RgGraph *g = NULL;
        rg_fixture_f1_campus(&g);
        rg_disable_edge_by_id(g, 3);
        rg_graph_reset(g);
        rg_graph_free(g);
    }
    RG_ASSERT_INT_EQ(rg_alloc_active_count(), base_allocs);

    /* Fault injection: fail allocation during graph creation */
    rg_alloc_fail_after(2); /* Fail on the 3rd allocation */
    RgGraph *fail_g = NULL;
    RgStatus st = rg_graph_create(12, 13, &fail_g);
    RG_ASSERT_INT_EQ(st, RG_ERR_ALLOC);
    RG_ASSERT(fail_g == NULL);
    RG_ASSERT_INT_EQ(rg_alloc_active_count(), base_allocs);
    rg_alloc_reset_stats();

    RG_TEST_END();
}

void test_rg_x_04(void) {
    RG_TEST_START("RG-X-04");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    int prev_edges = g->n_edges;

    /* Self-loop rejection */
    int out_id = -1;
    RgStatus st = rg_add_edge(g, 3, 3, &out_id);
    RG_ASSERT_INT_EQ(st, RG_ERR_SELF_LOOP);
    RG_ASSERT_INT_EQ(g->n_edges, prev_edges);

    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_x_05(void) {
    RG_TEST_START("RG-X-05");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    int prev_edges = g->n_edges;

    /* Duplicate edge rejection: e0 is (0, 1), attempt to add (1, 0) */
    int out_id = -1;
    RgStatus st = rg_add_edge(g, 1, 0, &out_id);
    RG_ASSERT_INT_EQ(st, RG_ERR_DUPLICATE_EDGE);
    RG_ASSERT_INT_EQ(g->n_edges, prev_edges);

    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_x_06(void) {
    RG_TEST_START("RG-X-06");
    RgGraph *g = NULL;
    RgStatus st1 = rg_graph_create(0, 10, &g);
    RG_ASSERT_INT_EQ(st1, RG_ERR_INVALID_PARAM);

    RgStatus st2 = rg_graph_create(-5, 10, &g);
    RG_ASSERT_INT_EQ(st2, RG_ERR_INVALID_PARAM);

    RgStatus st3 = rg_graph_create(5, -1, &g);
    RG_ASSERT_INT_EQ(st3, RG_ERR_INVALID_PARAM);

    RG_TEST_END();
}
