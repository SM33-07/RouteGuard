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
 * @brief Enable or disable an edge by ID.
 * @param edge_id 0-based edge ID.
 * @param active 1 to enable, 0 to disable.
 * @return 0 on success, negative on error.
 */
EMSCRIPTEN_KEEPALIVE
int rg_wasm_set_edge_active(int edge_id, int active);

/**
 * @brief Execute BFS traversal and compute route from src to dst.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for Adjacency Matrix, 0 for Adjacency List.
 * @return Pointer to static JSON buffer containing traversal order, route, and metrics.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_run_bfs(int src, int dst, int use_matrix);

/**
 * @brief Execute iterative DFS traversal from src.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID (to test reachability).
 * @param use_matrix 1 for Adjacency Matrix, 0 for Adjacency List.
 * @return Pointer to static JSON buffer containing traversal order, reachability, and metrics.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_run_dfs(int src, int dst, int use_matrix);

/**
 * @brief Find minimum-hop route from src to dst.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for Adjacency Matrix, 0 for Adjacency List.
 * @return Pointer to static JSON buffer containing route vertices, hop count, and metrics.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_find_route(int src, int dst, int use_matrix);

/**
 * @brief Compute connected components of current graph state.
 * @param use_matrix 1 for Adjacency Matrix, 0 for Adjacency List.
 * @return Pointer to static JSON buffer with component count and vertex component IDs.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_get_components(int use_matrix);

/**
 * @brief Get current network state summary for (src, dst).
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for Adjacency Matrix, 0 for Adjacency List.
 * @return Pointer to static JSON buffer with reachable count, component count, and route status.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_get_network_state(int src, int dst, int use_matrix);

/**
 * @brief Compute single-edge failure impact table across all edges.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @return Pointer to static JSON buffer containing 13 rows of impact data.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_get_failure_table(int src, int dst);

/**
 * @brief Export current graph state to an SVG string using the C11 exporter.
 * @param src Source vertex ID.
 * @param dst Destination vertex ID.
 * @param use_matrix 1 for Adjacency Matrix, 0 for Adjacency List.
 * @return Pointer to static buffer containing full SVG XML.
 */
EMSCRIPTEN_KEEPALIVE
const char *rg_wasm_export_svg(int src, int dst, int use_matrix);

#ifdef __cplusplus
}
#endif

#endif /* RG_WASM_BRIDGE_H */
