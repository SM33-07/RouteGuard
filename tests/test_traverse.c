#include "rg_test.h"
#include "rg_graph.h"
#include "rg_traverse.h"
#include "rg_fixture.h"

void test_rg_bfs_01(void) {
    RG_TEST_START("RG-BFS-01");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    RgTraverseResult *res = NULL;
    rg_traverse_result_create(g->n_vertices, &res);

    bfs_list(g, 0, res);

    int expected_order[] = {0, 1, 2, 11, 3, 4, 5, 6, 7, 8, 9, 10};
    RG_ASSERT_INT_EQ(res->order_len, 12);
    for (int i = 0; i < 12; i++) {
        RG_ASSERT_INT_EQ(res->order[i], expected_order[i]);
    }

    rg_traverse_result_free(res);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_bfs_02(void) {
    RG_TEST_START("RG-BFS-02");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    RgRoute *rt = NULL;
    rg_route_create(g->n_vertices, &rt);

    RgStatus st = rg_find_route(g, 0, 10, false, rt, NULL);
    RG_ASSERT_INT_EQ(st, RG_OK);
    RG_ASSERT(rt->found);
    RG_ASSERT_INT_EQ(rt->hop_count, 8);

    int expected_path[] = {0, 1, 3, 4, 6, 7, 8, 9, 10};
    for (int i = 0; i <= 8; i++) {
        RG_ASSERT_INT_EQ(rt->path[i], expected_path[i]);
    }

    rg_route_free(rt);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_bfs_03(void) {
    RG_TEST_START("RG-BFS-03");
    RgGraph *g = NULL;
    rg_fixture_f2_disconnected(&g);
    RgRoute *rt = NULL;
    rg_route_create(g->n_vertices, &rt);

    /* On F2, vertex 5 is isolated: path 0 -> 5 must fail with RG_NO_ROUTE */
    RgStatus st = rg_find_route(g, 0, 5, false, rt, NULL);
    RG_ASSERT_INT_EQ(st, RG_NO_ROUTE);
    RG_ASSERT(!rt->found);
    RG_ASSERT_INT_EQ(rt->hop_count, -1);

    rg_route_free(rt);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_bfs_04(void) {
    RG_TEST_START("RG-BFS-04");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    RgRoute *rt = NULL;
    rg_route_create(g->n_vertices, &rt);

    /* Source equals destination: 0 hops, single vertex path */
    RgStatus st = rg_find_route(g, 3, 3, false, rt, NULL);
    RG_ASSERT_INT_EQ(st, RG_OK);
    RG_ASSERT(rt->found);
    RG_ASSERT_INT_EQ(rt->hop_count, 0);
    RG_ASSERT_INT_EQ(rt->path[0], 3);

    rg_route_free(rt);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_dfs_01(void) {
    RG_TEST_START("RG-DFS-01");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    RgTraverseResult *res = NULL;
    rg_traverse_result_create(g->n_vertices, &res);

    dfs_list(g, 0, res);

    int expected_dfs[] = {0, 1, 3, 2, 4, 6, 5, 7, 8, 9, 10, 11};
    RG_ASSERT_INT_EQ(res->order_len, 12);
    for (int i = 0; i < 12; i++) {
        RG_ASSERT_INT_EQ(res->order[i], expected_dfs[i]);
    }

    rg_traverse_result_free(res);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_dfs_02(void) {
    RG_TEST_START("RG-DFS-02");
    RgGraph *g = NULL;
    rg_fixture_f2_disconnected(&g);

    RG_ASSERT(rg_reachable(g, 0, 1));
    RG_ASSERT(rg_reachable(g, 0, 2));
    RG_ASSERT(rg_reachable(g, 3, 4));
    RG_ASSERT(!rg_reachable(g, 0, 3));
    RG_ASSERT(!rg_reachable(g, 0, 5));
    RG_ASSERT(!rg_reachable(g, 0, 6));

    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_rep_01(void) {
    RG_TEST_START("RG-REP-01");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    RgTraverseResult *res_mat = NULL, *res_list = NULL;
    rg_traverse_result_create(g->n_vertices, &res_mat);
    rg_traverse_result_create(g->n_vertices, &res_list);

    bfs_matrix(g, 0, res_mat);
    bfs_list(g, 0, res_list);

    RG_ASSERT_INT_EQ(res_mat->order_len, res_list->order_len);
    for (int i = 0; i < g->n_vertices; i++) {
        RG_ASSERT_INT_EQ(res_mat->visited[i], res_list->visited[i]);
        RG_ASSERT_INT_EQ(res_mat->dist[i], res_list->dist[i]);
        RG_ASSERT_INT_EQ(res_mat->order[i], res_list->order[i]);
    }

    rg_traverse_result_free(res_mat);
    rg_traverse_result_free(res_list);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_rep_02(void) {
    RG_TEST_START("RG-REP-02");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);

    /* Toggle edge e5 (3-4) */
    rg_disable_edge_by_id(g, 5);

    RgTraverseResult *res_mat = NULL, *res_list = NULL;
    rg_traverse_result_create(g->n_vertices, &res_mat);
    rg_traverse_result_create(g->n_vertices, &res_list);

    bfs_matrix(g, 0, res_mat);
    bfs_list(g, 0, res_list);

    RG_ASSERT_INT_EQ(res_mat->order_len, res_list->order_len);
    for (int i = 0; i < g->n_vertices; i++) {
        RG_ASSERT_INT_EQ(res_mat->visited[i], res_list->visited[i]);
        RG_ASSERT_INT_EQ(res_mat->dist[i], res_list->dist[i]);
        RG_ASSERT_INT_EQ(res_mat->order[i], res_list->order[i]);
    }

    /* Both must find rerouted path of 8 hops via Block B */
    RgRoute *rt_mat = NULL, *rt_list = NULL;
    rg_route_create(g->n_vertices, &rt_mat);
    rg_route_create(g->n_vertices, &rt_list);
    rg_find_route(g, 0, 10, true, rt_mat, NULL);
    rg_find_route(g, 0, 10, false, rt_list, NULL);

    RG_ASSERT_INT_EQ(rt_mat->hop_count, 8);
    RG_ASSERT_INT_EQ(rt_list->hop_count, 8);
    for (int i = 0; i <= 8; i++) {
        RG_ASSERT_INT_EQ(rt_mat->path[i], rt_list->path[i]);
    }

    rg_route_free(rt_mat);
    rg_route_free(rt_list);
    rg_traverse_result_free(res_mat);
    rg_traverse_result_free(res_list);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_x_08(void) {
    RG_TEST_START("RG-X-08");
    /* Verify iterative DFS equals recursive DFS on F1, F2, F4, F5 */
    RgGraph *fixtures[4];
    rg_fixture_f1_campus(&fixtures[0]);
    rg_fixture_f2_disconnected(&fixtures[1]);
    rg_fixture_f4_chain(&fixtures[2]);
    rg_fixture_f5_cycle4(&fixtures[3]);

    for (int f = 0; f < 4; f++) {
        RgGraph *g = fixtures[f];
        RgTraverseResult *iter_res = NULL, *rec_res = NULL;
        rg_traverse_result_create(g->n_vertices, &iter_res);
        rg_traverse_result_create(g->n_vertices, &rec_res);

        dfs_list(g, 0, iter_res);
        dfs_list_recursive(g, 0, rec_res);

        RG_ASSERT_INT_EQ(iter_res->order_len, rec_res->order_len);
        for (int i = 0; i < iter_res->order_len; i++) {
            RG_ASSERT_INT_EQ(iter_res->order[i], rec_res->order[i]);
        }

        rg_traverse_result_free(iter_res);
        rg_traverse_result_free(rec_res);
        rg_graph_free(g);
    }

    RG_TEST_END();
}

void test_rg_x_09(void) {
    RG_TEST_START("RG-X-09");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);
    RgTraverseResult *res_mat = NULL, *res_list = NULL;
    rg_traverse_result_create(g->n_vertices, &res_mat);
    rg_traverse_result_create(g->n_vertices, &res_list);

    bfs_matrix(g, 0, res_mat);
    bfs_list(g, 0, res_list);

    /* Dedicated slot count check: Matrix scans 144 slots (V*V), List scans 26 slots (2E) */
    RG_ASSERT_INT_EQ(res_mat->metrics.neighbor_slots_examined, 144);
    RG_ASSERT_INT_EQ(res_list->metrics.neighbor_slots_examined, 26);

    rg_traverse_result_free(res_mat);
    rg_traverse_result_free(res_list);
    rg_graph_free(g);
    RG_TEST_END();
}
