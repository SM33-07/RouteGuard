#ifndef RG_ANALYSIS_H
#define RG_ANALYSIS_H

#include "rg_graph.h"
#include <stdbool.h>

/**
 * @file rg_analysis.h
 * @brief Connected components, impact summaries, and single-failure analysis.
 */

typedef struct {
    int reachable_src_count;    /* Number of vertices reachable from source */
    int component_count;        /* Total connected components in graph */
    int route_hops;             /* Shortest-hop distance between src and dst, or -1 */
    bool route_found;
} RgNetworkState;

typedef struct {
    RgNetworkState before;
    RgNetworkState after;
} RgImpactSummary;

typedef struct {
    int edge_id;
    int u;
    int v;
    bool reachable;
    int hop_count;              /* -1 if unreachable */
    int reachable_src_count;
    int component_count;
} RgFailureRow;

typedef struct {
    int n_rows;
    RgFailureRow *rows;
} RgFailureTable;

/**
 * @brief Computes connected components using repeated matrix BFS.
 * @param out_component_ids Array of size n_vertices, or NULL.
 * @return Total number of connected components.
 */
int rg_connected_components_matrix(const RgGraph *graph, int *out_component_ids);

/**
 * @brief Computes connected components using repeated list BFS.
 * @param out_component_ids Array of size n_vertices, or NULL.
 * @return Total number of connected components.
 */
int rg_connected_components_list(const RgGraph *graph, int *out_component_ids);

/**
 * @brief Snapshot the current routing and connectivity state for (src, dst).
 */
RgStatus rg_get_network_state(const RgGraph *graph, int src, int dst, bool use_matrix, RgNetworkState *out_state);

/**
 * @brief Allocates a failure table structure for n_edges rows.
 */
RgStatus rg_failure_table_create(int n_edges, RgFailureTable **out_table);

/**
 * @brief Deallocates a failure table.
 */
void rg_failure_table_free(RgFailureTable *table);

/**
 * @brief Brute-force repeated BFS single-failure analysis table: O(E * (V + E)).
 *
 * For each edge, temporarily disables it, evaluates reachability and components,
 * and restores the edge. Asserts graph state before and after the loop is byte-identical.
 */
RgStatus rg_compute_failure_table(RgGraph *graph, int src, int dst, RgFailureTable *table);

#endif /* RG_ANALYSIS_H */
