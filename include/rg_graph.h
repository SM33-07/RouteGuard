#ifndef RG_GRAPH_H
#define RG_GRAPH_H

#include <stdbool.h>

/**
 * @file rg_graph.h
 * @brief Undirected, unweighted graph model with dual matrix and sorted adjacency list representations.
 */

typedef enum {
    RG_OK = 0,
    RG_NOOP = 1,
    RG_NO_ROUTE = 2,
    RG_ERR_INVALID_PARAM = -1,
    RG_ERR_SELF_LOOP = -2,
    RG_ERR_DUPLICATE_EDGE = -3,
    RG_ERR_ALLOC = -4,
    RG_ERR_NOT_FOUND = -5
} RgStatus;

typedef struct {
    int u;
    int v;
    bool active;
} RgEdge;

typedef struct RgAdjNode {
    int neighbor;
    int edge_id;
    struct RgAdjNode *next;
} RgAdjNode;

typedef struct {
    int n_vertices;
    int n_edges;
    int max_edges;
    char (*labels)[32];
    int *coords_x;
    int *coords_y;
    RgEdge *edges;            /* Canonical edge array: active state lives ONLY here */
    int *matrix;              /* n * n: stores edge_id (0..n_edges-1) or -1 for no edge */
    RgAdjNode **adj_list;     /* Array of singly linked lists, sorted ascending by neighbor */
} RgGraph;

/**
 * @brief Allocates and initializes an RgGraph.
 * @note Ownership: The caller owns the graph and must release it with rg_graph_free().
 */
RgStatus rg_graph_create(int n_vertices, int max_edges, RgGraph **out_graph);

/**
 * @brief Deallocates all graph memory, including labels, arrays, and adjacency list nodes.
 */
void rg_graph_free(RgGraph *graph);

/**
 * @brief Sets optional label and display coordinates for a vertex.
 */
RgStatus rg_set_vertex(RgGraph *graph, int v, const char *label, int x, int y);

/**
 * @brief Adds an undirected edge between u and v.
 *
 * Validates endpoints, checks for self-loops and duplicate edges.
 * Inserts in sorted ascending order in adjacency lists and updates matrix with the edge ID.
 * On failure, leaves graph completely unmodified.
 */
RgStatus rg_add_edge(RgGraph *graph, int u, int v, int *out_edge_id);

/**
 * @brief Disables an edge by ID. Harmless no-op returning RG_NOOP if already disabled.
 */
RgStatus rg_disable_edge_by_id(RgGraph *graph, int edge_id);

/**
 * @brief Enables an edge by ID. Harmless no-op returning RG_NOOP if already active.
 */
RgStatus rg_enable_edge_by_id(RgGraph *graph, int edge_id);

/**
 * @brief Disables an edge by its endpoints.
 */
RgStatus rg_disable_edge_by_endpoints(RgGraph *graph, int u, int v);

/**
 * @brief Enables an edge by its endpoints.
 */
RgStatus rg_enable_edge_by_endpoints(RgGraph *graph, int u, int v);

/**
 * @brief Restores all edges in the graph to active state.
 */
RgStatus rg_graph_reset(RgGraph *graph);

/**
 * @brief Retrieves edge ID between u and v, or -1 if no edge exists.
 */
int rg_get_edge_id(const RgGraph *graph, int u, int v);

/**
 * @brief Checks if edge with given ID is currently active.
 */
bool rg_is_edge_active(const RgGraph *graph, int edge_id);

/**
 * @brief Human-readable status message for logging and assertions.
 */
const char *rg_status_string(RgStatus status);

#endif /* RG_GRAPH_H */
