#include "rg_test.h"
#include "rg_graph.h"
#include "rg_traverse.h"
#include "rg_fixture.h"
#include "rg_analysis.h"

void test_rg_met_01(void) {
    RG_TEST_START("RG-MET-01");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);

    /* Run fixed fixture twice and assert identical operation counters */
    RgTraverseResult *bfs_run1 = NULL, *bfs_run2 = NULL;
    RgTraverseResult *dfs_run1 = NULL, *dfs_run2 = NULL;
    rg_traverse_result_create(g->n_vertices, &bfs_run1);
    rg_traverse_result_create(g->n_vertices, &bfs_run2);
    rg_traverse_result_create(g->n_vertices, &dfs_run1);
    rg_traverse_result_create(g->n_vertices, &dfs_run2);

    bfs_list(g, 0, bfs_run1);
    bfs_list(g, 0, bfs_run2);

    dfs_list(g, 0, dfs_run1);
    dfs_list(g, 0, dfs_run2);

    /* BFS counter determinism */
    RG_ASSERT_INT_EQ(bfs_run1->metrics.vertices_processed, bfs_run2->metrics.vertices_processed);
    RG_ASSERT_INT_EQ(bfs_run1->metrics.neighbor_slots_examined, bfs_run2->metrics.neighbor_slots_examined);
    RG_ASSERT_INT_EQ(bfs_run1->metrics.queue_enqueues, bfs_run2->metrics.queue_enqueues);
    RG_ASSERT_INT_EQ(bfs_run1->metrics.queue_dequeues, bfs_run2->metrics.queue_dequeues);
    RG_ASSERT_INT_EQ(bfs_run1->metrics.edges_skipped_inactive, bfs_run2->metrics.edges_skipped_inactive);

    /* DFS counter determinism */
    RG_ASSERT_INT_EQ(dfs_run1->metrics.vertices_processed, dfs_run2->metrics.vertices_processed);
    RG_ASSERT_INT_EQ(dfs_run1->metrics.neighbor_slots_examined, dfs_run2->metrics.neighbor_slots_examined);
    RG_ASSERT_INT_EQ(dfs_run1->metrics.stack_pushes, dfs_run2->metrics.stack_pushes);
    RG_ASSERT_INT_EQ(dfs_run1->metrics.stack_pops, dfs_run2->metrics.stack_pops);
    RG_ASSERT_INT_EQ(dfs_run1->metrics.stale_pops, dfs_run2->metrics.stale_pops);
    RG_ASSERT_INT_EQ(dfs_run1->metrics.edges_skipped_inactive, dfs_run2->metrics.edges_skipped_inactive);

    rg_traverse_result_free(bfs_run1);
    rg_traverse_result_free(bfs_run2);
    rg_traverse_result_free(dfs_run1);
    rg_traverse_result_free(dfs_run2);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_x_03(void) {
    RG_TEST_START("RG-X-03");
    RgGraph *g = NULL;
    rg_fixture_f1_campus(&g);

    RgTraverseResult *bfs_l = NULL, *bfs_m = NULL;
    RgTraverseResult *dfs_l = NULL;
    rg_traverse_result_create(g->n_vertices, &bfs_l);
    rg_traverse_result_create(g->n_vertices, &bfs_m);
    rg_traverse_result_create(g->n_vertices, &dfs_l);

    bfs_list(g, 0, bfs_l);
    bfs_matrix(g, 0, bfs_m);
    dfs_list(g, 0, dfs_l);

    /* Independently derived F1 Reference Counters:
     * - BFS List: Processed=12, Slots=26, Enqueues=12, Dequeues=12
     * - BFS Matrix: Slots=144 (12 * 12)
     * - DFS List: Pushes=14, Pops=14, StalePops=2, Processed=12, Slots=26
     */
    RG_ASSERT_INT_EQ(bfs_l->metrics.vertices_processed, 12);
    RG_ASSERT_INT_EQ(bfs_l->metrics.neighbor_slots_examined, 26);
    RG_ASSERT_INT_EQ(bfs_l->metrics.queue_enqueues, 12);
    RG_ASSERT_INT_EQ(bfs_l->metrics.queue_dequeues, 12);

    RG_ASSERT_INT_EQ(bfs_m->metrics.neighbor_slots_examined, 144);

    RG_ASSERT_INT_EQ(dfs_l->metrics.vertices_processed, 12);
    RG_ASSERT_INT_EQ(dfs_l->metrics.neighbor_slots_examined, 26);
    RG_ASSERT_INT_EQ(dfs_l->metrics.stack_pushes, 14);
    RG_ASSERT_INT_EQ(dfs_l->metrics.stack_pops, 14);
    RG_ASSERT_INT_EQ(dfs_l->metrics.stale_pops, 2);

    rg_traverse_result_free(bfs_l);
    rg_traverse_result_free(bfs_m);
    rg_traverse_result_free(dfs_l);
    rg_graph_free(g);
    RG_TEST_END();
}

void test_rg_x_12_campus(void) {
    RG_TEST_START("RG-X-12");
    RgGraph *g = NULL;
    RgStatus st = rg_fixture_campus_muj(&g);
    RG_ASSERT_INT_EQ(st, RG_OK);
    RG_ASSERT(g != NULL);
    RG_ASSERT_INT_EQ(g->n_vertices, 12);
    RG_ASSERT_INT_EQ(g->n_edges, 15);

    /* 1. Labels and descriptions verification */
    RG_ASSERT_STR_EQ(g->labels[0], "Main Gate");
    RG_ASSERT_STR_EQ(g->labels[1], "Dome Building");
    RG_ASSERT_STR_EQ(g->labels[6], "Academic Block 3");
    RG_ASSERT_STR_EQ(g->labels[7], "Lecture Hall Complex");
    RG_ASSERT_STR_EQ(g->labels[9], "Football Ground");
    RG_ASSERT_STR_EQ(rg_fixture_campus_venue_description(0),
                     "Main Gate — Primary campus arrival & visitor entry");
    RG_ASSERT_STR_EQ(rg_fixture_campus_venue_description(1),
                     "Dome Building — Library, administration, services (Landmark 1)");
    RG_ASSERT_STR_EQ(rg_fixture_campus_venue_description(6),
                     "Academic Block 3 — Arts, media and specialist spaces (Landmark 6)");
    RG_ASSERT_STR_EQ(rg_fixture_campus_venue_description(7),
                     "Lecture Hall Complex — Large teaching halls (Landmark 5)");
    RG_ASSERT_STR_EQ(rg_fixture_campus_venue_description(9),
                     "Football Ground — Sports grounds (Landmark 7)");

    /* 2. Connected components in baseline */
    int comp_map[12];
    int c_count = rg_connected_components_list(g, comp_map);
    RG_ASSERT_INT_EQ(c_count, 1);

    /* 3. BFS Baseline Traversal from 0 */
    RgTraverseResult *bfs_l = NULL, *bfs_m = NULL;
    rg_traverse_result_create(g->n_vertices, &bfs_l);
    rg_traverse_result_create(g->n_vertices, &bfs_m);

    bfs_list(g, 0, bfs_l);
    bfs_matrix(g, 0, bfs_m);

    RG_ASSERT_INT_EQ(bfs_l->metrics.vertices_processed, 12);
    RG_ASSERT_INT_EQ(bfs_l->metrics.neighbor_slots_examined, 30);
    RG_ASSERT_INT_EQ(bfs_l->metrics.queue_enqueues, 12);
    RG_ASSERT_INT_EQ(bfs_l->metrics.queue_dequeues, 12);
    RG_ASSERT_INT_EQ(bfs_l->metrics.edges_skipped_inactive, 0);

    RG_ASSERT_INT_EQ(bfs_m->metrics.vertices_processed, 12);
    RG_ASSERT_INT_EQ(bfs_m->metrics.neighbor_slots_examined, 144);

    int expected_bfs_order[12] = {0, 1, 2, 3, 4, 5, 6, 8, 7, 11, 9, 10};
    for (int i = 0; i < 12; i++) {
        RG_ASSERT_INT_EQ(bfs_l->order[i], expected_bfs_order[i]);
        RG_ASSERT_INT_EQ(bfs_m->order[i], expected_bfs_order[i]);
    }

    /* 4. Baseline Minimum-Hop Routes */
    RgRoute *route = NULL;
    rg_route_create(g->n_vertices, &route);

    /* Route 0 -> 6 (Academic Block 3): 4 hops */
    st = rg_find_route(g, 0, 6, false, route, NULL);
    RG_ASSERT_INT_EQ(st, RG_OK);
    RG_ASSERT(route->found);
    RG_ASSERT_INT_EQ(route->hop_count, 4);
    int expected_route_0_6[5] = {0, 1, 2, 3, 6};
    for (int i = 0; i <= 4; i++) {
        RG_ASSERT_INT_EQ(route->path[i], expected_route_0_6[i]);
    }

    /* Route 0 -> 9 (Football Ground): 5 hops */
    st = rg_find_route(g, 0, 9, false, route, NULL);
    RG_ASSERT_INT_EQ(st, RG_OK);
    RG_ASSERT(route->found);
    RG_ASSERT_INT_EQ(route->hop_count, 5);
    int expected_route_0_9[6] = {0, 1, 2, 5, 7, 9};
    for (int i = 0; i <= 5; i++) {
        RG_ASSERT_INT_EQ(route->path[i], expected_route_0_9[i]);
    }

    /* 5. Alternative Route Demonstration: Disable redundant edge e6 (3 <-> 6) */
    st = rg_disable_edge_by_id(g, 6);
    RG_ASSERT_INT_EQ(st, RG_OK);

    /* Route 0 -> 6 reroutes via Old Mess and LHC (hops=5) */
    st = rg_find_route(g, 0, 6, false, route, NULL);
    RG_ASSERT_INT_EQ(st, RG_OK);
    RG_ASSERT(route->found);
    RG_ASSERT_INT_EQ(route->hop_count, 5);
    int expected_reroute_0_6[6] = {0, 1, 2, 5, 7, 6};
    for (int i = 0; i <= 5; i++) {
        RG_ASSERT_INT_EQ(route->path[i], expected_reroute_0_6[i]);
    }

    /* Re-enable e6 */
    st = rg_enable_edge_by_id(g, 6);
    RG_ASSERT_INT_EQ(st, RG_OK);

    /* 6. Critical Link Severance: Disable e12 (7 <-> 9, LHC <-> Football Ground) */
    st = rg_disable_edge_by_id(g, 12);
    RG_ASSERT_INT_EQ(st, RG_OK);

    /* Route 0 -> 9 is now disconnected */
    st = rg_find_route(g, 0, 9, false, route, NULL);
    RG_ASSERT_INT_EQ(st, RG_NO_ROUTE);
    RG_ASSERT(!route->found);
    RG_ASSERT_INT_EQ(route->hop_count, -1);

    /* Component count becomes 2 */
    c_count = rg_connected_components_list(g, comp_map);
    RG_ASSERT_INT_EQ(c_count, 2);

    /* Reachability from 0: exactly 10 vertices */
    rg_traverse_result_free(bfs_l);
    rg_traverse_result_create(g->n_vertices, &bfs_l);
    bfs_list(g, 0, bfs_l);
    RG_ASSERT_INT_EQ(bfs_l->metrics.vertices_processed, 10);
    RG_ASSERT_INT_EQ(bfs_l->metrics.neighbor_slots_examined, 27);
    RG_ASSERT_INT_EQ(bfs_l->metrics.edges_skipped_inactive, 1);

    rg_traverse_result_free(bfs_m);
    rg_traverse_result_create(g->n_vertices, &bfs_m);
    bfs_matrix(g, 0, bfs_m);
    RG_ASSERT_INT_EQ(bfs_m->metrics.vertices_processed, 10);
    RG_ASSERT_INT_EQ(bfs_m->metrics.neighbor_slots_examined, 120);

    /* 7. Reset restores baseline */
    st = rg_graph_reset(g);
    RG_ASSERT_INT_EQ(st, RG_OK);

    c_count = rg_connected_components_list(g, comp_map);
    RG_ASSERT_INT_EQ(c_count, 1);

    st = rg_find_route(g, 0, 9, false, route, NULL);
    RG_ASSERT_INT_EQ(st, RG_OK);
    RG_ASSERT(route->found);
    RG_ASSERT_INT_EQ(route->hop_count, 5);

    /* Cleanup */
    rg_route_free(route);
    rg_traverse_result_free(bfs_l);
    rg_traverse_result_free(bfs_m);
    rg_graph_free(g);
    RG_TEST_END();
}


