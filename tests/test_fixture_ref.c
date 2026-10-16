#include "rg_test.h"
#include "rg_graph.h"
#include "rg_traverse.h"
#include "rg_fixture.h"

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
