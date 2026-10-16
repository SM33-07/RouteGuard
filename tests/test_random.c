#include "rg_test.h"
#include "rg_graph.h"
#include "rg_traverse.h"
#include "rg_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_ORACLE_VERTICES 32

/* Fixed-seed Linear Congruential Generator (LCG) */
static uint32_t s_lcg_state = 20261017U;

static uint32_t lcg_rand(void) {
    s_lcg_state = (1103515245U * s_lcg_state + 12345U) & 0x7FFFFFFFU;
    return s_lcg_state;
}

static int lcg_range(int min_val, int max_val) {
    if (min_val >= max_val) return min_val;
    uint32_t r = lcg_rand();
    return min_val + (int)(r % (uint32_t)(max_val - min_val + 1));
}

/*
 * Independent Oracle 1: Shortest Hop Distance via Repeated Relaxation
 * Does NOT use queues, stacks, or traversal data structures.
 * Repeatedly relaxes distances until convergence (Bellman-Ford on unweighted graph).
 */
static void oracle_shortest_hops(const RgGraph *g, int src, int *dist_out) {
    for (int i = 0; i < g->n_vertices; i++) {
        dist_out[i] = (i == src) ? 0 : -1;
    }

    bool changed = true;
    for (int iter = 0; iter < g->n_vertices && changed; iter++) {
        changed = false;
        for (int e = 0; e < g->n_edges; e++) {
            if (!g->edges[e].active) continue;
            int u = g->edges[e].u;
            int v = g->edges[e].v;

            /* Relax u -> v */
            if (dist_out[u] != -1) {
                if (dist_out[v] == -1 || dist_out[u] + 1 < dist_out[v]) {
                    dist_out[v] = dist_out[u] + 1;
                    changed = true;
                }
            }
            /* Relax v -> u */
            if (dist_out[v] != -1) {
                if (dist_out[u] == -1 || dist_out[v] + 1 < dist_out[u]) {
                    dist_out[u] = dist_out[v] + 1;
                    changed = true;
                }
            }
        }
    }
}

/*
 * Independent Oracle 2: Connected Components via Label Propagation
 * Does NOT use BFS, DFS, or queue/stack structures.
 * Propagates minimum vertex labels until stability.
 */
static int oracle_component_count(const RgGraph *g) {
    int labels[MAX_ORACLE_VERTICES];
    for (int i = 0; i < g->n_vertices; i++) {
        labels[i] = i;
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (int e = 0; e < g->n_edges; e++) {
            if (!g->edges[e].active) continue;
            int u = g->edges[e].u;
            int v = g->edges[e].v;
            int min_lbl = (labels[u] < labels[v]) ? labels[u] : labels[v];
            if (labels[u] != min_lbl) {
                labels[u] = min_lbl;
                changed = true;
            }
            if (labels[v] != min_lbl) {
                labels[v] = min_lbl;
                changed = true;
            }
        }
    }

    /* Count unique labels */
    bool seen[MAX_ORACLE_VERTICES] = {false};
    int count = 0;
    for (int i = 0; i < g->n_vertices; i++) {
        int root = labels[i];
        if (!seen[root]) {
            seen[root] = true;
            count++;
        }
    }
    return count;
}

void test_rg_x_10_differential(void) {
    RG_TEST_START("RG-X-10");

    s_lcg_state = 20261017U;
    uint32_t initial_seed = s_lcg_state;

    const int NUM_GRAPHS = 300;
    long long total_comparisons = 0;
    int mismatches = 0;

    printf("\n  [Differential Oracle] Fixed LCG Seed = %u | Target Graphs = %d\n", initial_seed, NUM_GRAPHS);

    for (int g_idx = 0; g_idx < NUM_GRAPHS; g_idx++) {
        int n = lcg_range(1, 12);
        int max_possible_edges = n * (n - 1) / 2;
        int target_edges = (max_possible_edges > 0) ? lcg_range(0, max_possible_edges) : 0;

        RgGraph *g = NULL;
        RgStatus st = rg_graph_create(n, (target_edges > 0 ? target_edges : 1), &g);
        RG_ASSERT(st == RG_OK);

        for (int i = 0; i < n; i++) {
            char lbl[32];
            snprintf(lbl, sizeof(lbl), "V%d", i);
            rg_set_vertex(g, i, lbl, i * 50, (g_idx % 5) * 40);
        }

        /* Generate random unique undirected edges */
        int edges_added = 0;
        int attempts = 0;
        while (edges_added < target_edges && attempts < target_edges * 10) {
            attempts++;
            int u = lcg_range(0, n - 1);
            int v = lcg_range(0, n - 1);
            if (u == v) continue;
            int edge_id = -1;
            if (rg_add_edge(g, u, v, &edge_id) == RG_OK) {
                edges_added++;
            }
        }

        /* Randomly disable a subset of edges (approx 25% probability) */
        for (int e = 0; e < g->n_edges; e++) {
            if (lcg_range(0, 3) == 0) {
                rg_disable_edge_by_id(g, e);
            }
        }

        /* 1. Component count differential test */
        int comp_matrix = rg_connected_components_matrix(g, NULL);
        int comp_list = rg_connected_components_list(g, NULL);
        int comp_oracle = oracle_component_count(g);

        total_comparisons++;
        if (comp_matrix != comp_list || comp_matrix != comp_oracle) {
            mismatches++;
        }
        RG_ASSERT(comp_matrix == comp_list);
        RG_ASSERT(comp_matrix == comp_oracle);

        /* 2. Traversal and route distance differential tests for every source */
        RgTraverseResult *res_m = NULL;
        RgTraverseResult *res_l = NULL;
        RgTraverseResult *res_dfs = NULL;
        RgTraverseResult *res_rec = NULL;

        rg_traverse_result_create(n, &res_m);
        rg_traverse_result_create(n, &res_l);
        rg_traverse_result_create(n, &res_dfs);
        rg_traverse_result_create(n, &res_rec);

        int oracle_dist[MAX_ORACLE_VERTICES];

        for (int src = 0; src < n; src++) {
            /* Oracle calculation */
            oracle_shortest_hops(g, src, oracle_dist);

            /* BFS Matrix & List */
            bfs_matrix(g, src, res_m);
            bfs_list(g, src, res_l);

            /* Verify BFS distances and reachability against relaxation oracle */
            for (int v = 0; v < n; v++) {
                total_comparisons += 3;
                if (res_m->dist[v] != res_l->dist[v] ||
                    res_m->dist[v] != oracle_dist[v] ||
                    res_m->visited[v] != (oracle_dist[v] != -1) ||
                    res_l->visited[v] != (oracle_dist[v] != -1)) {
                    mismatches++;
                }
                RG_ASSERT(res_m->dist[v] == res_l->dist[v]);
                RG_ASSERT(res_m->dist[v] == oracle_dist[v]);
                RG_ASSERT(res_m->visited[v] == (oracle_dist[v] != -1));
                RG_ASSERT(res_l->visited[v] == (oracle_dist[v] != -1));
            }

            /* Iterative DFS vs Recursive DFS reference order */
            dfs_list(g, src, res_dfs);
            dfs_list_recursive(g, src, res_rec);

            total_comparisons += 2;
            if (res_dfs->order_len != res_rec->order_len) {
                mismatches++;
            }
            RG_ASSERT(res_dfs->order_len == res_rec->order_len);

            for (int i = 0; i < res_dfs->order_len; i++) {
                total_comparisons++;
                if (res_dfs->order[i] != res_rec->order[i]) {
                    mismatches++;
                }
                RG_ASSERT(res_dfs->order[i] == res_rec->order[i]);
            }
        }

        rg_traverse_result_free(res_m);
        rg_traverse_result_free(res_l);
        rg_traverse_result_free(res_dfs);
        rg_traverse_result_free(res_rec);
        rg_graph_free(g);
    }

    printf("  [Differential Oracle] Completed %d graphs | %lld total comparisons | %d mismatches\n",
           NUM_GRAPHS, total_comparisons, mismatches);
    RG_ASSERT(mismatches == 0);
    RG_TEST_END();
}
