#ifndef RG_WASM_BRIDGE_H
#define RG_WASM_BRIDGE_H

#include "rg_graph.h"
#include "rg_traverse.h"
#include "rg_analysis.h"
#include "rg_fixture.h"
#include "rg_svg.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize or re-initialize the WASM global graph instance with fixture F1.
 * @return 0 on success, non-zero on failure.
 */
EMSCRIPTEN_KEEPALIVE
int rg_wasm_init(void);

/**
 * @brief Reset the graph, re-enabling all edges.
 * @return 0 on success, non-zero on failure.
 */
EMSCRIPTEN_KEEPALIVE
int rg_wasm_reset(void);

/**
 * @brief Get JSON representation of vertices and edges.
 * @return Pointer to static JSON buffer.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_get_graph_json(void);

/**
 * @brief Set the active state of an edge by its canonical ID.
 * @param edge_id The edge ID (0 to n_edges-1).
 * @param active 1 for active, 0 for disabled.
 * @return 0 on success, -1 on error.
 */
EMSCRIPTEN_KEEPALIVE
int rg_wasm_set_edge_active(int edge_id, int active);

/**
 * @brief Run BFS from source to destination.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for matrix, 0 for list.
 * @return JSON string with traversal order, route, hop count, and metrics.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_run_bfs(int src, int dst, int use_matrix);

/**
 * @brief Run DFS from source to destination.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for matrix, 0 for list.
 * @return JSON string with DFS order, reachability, stale pops, and metrics.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_run_dfs(int src, int dst, int use_matrix);

/**
 * @brief Find the minimum-hop route between source and destination.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for matrix, 0 for list.
 * @return JSON string with path array, hop count, and status.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_find_route(int src, int dst, int use_matrix);

/**
 * @brief Compute connected components.
 * @param use_matrix 1 for matrix, 0 for list.
 * @return JSON string with component count and component labels per vertex.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_get_components(int use_matrix);

/**
 * @brief Get current network resilience state for source-destination pair.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for matrix, 0 for list.
 * @return JSON string with reachable count, component count, route status, and hops.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_get_network_state(int src, int dst, int use_matrix);

/**
 * @brief Generate single-edge failure impact table as JSON.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @return JSON string with array of edge failure rows.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_get_failure_table(int src, int dst);

/**
 * @brief Export current graph as SVG.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for matrix, 0 for list.
 * @return Pointer to static SVG buffer.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_export_svg(int src, int dst, int use_matrix);

#ifdef __cplusplus
}
#endif

#endif /* RG_WASM_BRIDGE_H */
