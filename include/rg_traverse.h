#ifndef RG_TRAVERSE_H
#define RG_TRAVERSE_H

#include "rg_graph.h"
#include <stdbool.h>

/**
 * @file rg_traverse.h
 * @brief Graph traversal algorithms (BFS, DFS matrix & list), route reconstruction, and reachability.
 */

typedef struct {
    int vertices_processed;         /* Dequeued in BFS, or popped-and-not-stale in DFS */
    int neighbor_slots_examined;    /* Matrix: all inspected columns; List: all inspected nodes */
    int edges_skipped_inactive;     /* Encountered edge is disabled in edges[] */
    int queue_enqueues;
    int queue_dequeues;
    int stack_pushes;
    int stack_pops;
    int stale_pops;
    double elapsed_ms;              /* Informational only, never asserted */
} RgMetrics;

typedef struct {
    int *order;        /* Visit order array (up to n_vertices) */
    int order_len;     /* Total visited vertices in traversal */
    bool *visited;     /* Visited flags, size = n_vertices */
    int *dist;         /* Hop distance from source, -1 if unreachable */
    int *pred;         /* Predecessor vertex, -1 if none/start */
    RgMetrics metrics;
} RgTraverseResult;

typedef struct {
    int *path;         /* Sequence of vertex IDs from src to dst */
    int hop_count;     /* Edge count (hops). 0 if src == dst, -1 if unreachable */
    bool found;        /* True if destination is reachable from source */
} RgRoute;

RgStatus rg_traverse_result_create(int n_vertices, RgTraverseResult **out_res);
void rg_traverse_result_free(RgTraverseResult *res);

RgStatus rg_route_create(int n_vertices, RgRoute **out_route);
void rg_route_free(RgRoute *route);

/**
 * @brief BFS on adjacency matrix representation.
 */
RgStatus bfs_matrix(const RgGraph *graph, int src, RgTraverseResult *res);

/**
 * @brief BFS on adjacency list representation.
 */
RgStatus bfs_list(const RgGraph *graph, int src, RgTraverseResult *res);

/**
 * @brief Iterative DFS on adjacency matrix representation.
 */
RgStatus dfs_matrix(const RgGraph *graph, int src, RgTraverseResult *res);

/**
 * @brief Iterative DFS on adjacency list representation.
 */
RgStatus dfs_list(const RgGraph *graph, int src, RgTraverseResult *res);

/**
 * @brief Recursive reference DFS on adjacency list (test-only cross check).
 */
RgStatus dfs_list_recursive(const RgGraph *graph, int src, RgTraverseResult *res);

/**
 * @brief Reconstructs minimum-hop path from BFS predecessors.
 */
RgStatus rg_find_route(const RgGraph *graph, int src, int dst, bool use_matrix, RgRoute *out_route, RgMetrics *out_metrics);

/**
 * @brief Checks if dst is reachable from src using DFS.
 */
bool rg_reachable(const RgGraph *graph, int src, int dst);

#endif /* RG_TRAVERSE_H */
