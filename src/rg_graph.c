#include "rg_graph.h"
#include "rg_alloc.h"
#include <string.h>

RgStatus rg_graph_create(int n_vertices, int max_edges, RgGraph **out_graph) {
    if (out_graph == NULL || n_vertices < 1 || max_edges < 0) {
        return RG_ERR_INVALID_PARAM;
    }

    RgGraph *g = rg_malloc(sizeof(RgGraph));
    if (g == NULL) {
        return RG_ERR_ALLOC;
    }

    g->n_vertices = n_vertices;
    g->n_edges = 0;
    g->max_edges = max_edges;

    g->labels = rg_calloc((size_t)n_vertices, 32);
    g->coords_x = rg_calloc((size_t)n_vertices, sizeof(int));
    g->coords_y = rg_calloc((size_t)n_vertices, sizeof(int));
    g->edges = (max_edges > 0) ? rg_calloc((size_t)max_edges, sizeof(RgEdge)) : NULL;
    g->matrix = rg_malloc((size_t)(n_vertices * n_vertices) * sizeof(int));
    g->adj_list = rg_calloc((size_t)n_vertices, sizeof(RgAdjNode *));

    if (g->labels == NULL || g->coords_x == NULL || g->coords_y == NULL ||
        (max_edges > 0 && g->edges == NULL) || g->matrix == NULL || g->adj_list == NULL) {
        rg_graph_free(g);
        return RG_ERR_ALLOC;
    }

    /* Initialize adjacency matrix with -1 (no edge) */
    for (int i = 0; i < n_vertices * n_vertices; i++) {
        g->matrix[i] = -1;
    }

    *out_graph = g;
    return RG_OK;
}

void rg_graph_free(RgGraph *graph) {
    if (graph == NULL) {
        return;
    }

    if (graph->adj_list != NULL) {
        for (int i = 0; i < graph->n_vertices; i++) {
            RgAdjNode *curr = graph->adj_list[i];
            while (curr != NULL) {
                RgAdjNode *next = curr->next;
                rg_free(curr);
                curr = next;
            }
        }
        rg_free(graph->adj_list);
    }

    rg_free(graph->matrix);
    rg_free(graph->edges);
    rg_free(graph->coords_y);
    rg_free(graph->coords_x);
    rg_free(graph->labels);
    rg_free(graph);
}

RgStatus rg_set_vertex(RgGraph *graph, int v, const char *label, int x, int y) {
    if (graph == NULL || v < 0 || v >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    if (label != NULL) {
        strncpy(graph->labels[v], label, 31);
        graph->labels[v][31] = '\0';
    }
    graph->coords_x[v] = x;
    graph->coords_y[v] = y;
    return RG_OK;
}

RgStatus rg_add_edge(RgGraph *graph, int u, int v, int *out_edge_id) {
    if (graph == NULL || u < 0 || u >= graph->n_vertices || v < 0 || v >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    if (u == v) {
        return RG_ERR_SELF_LOOP;
    }
    /* Duplicate check: matrix stores existing edge_id or -1 */
    if (graph->matrix[u * graph->n_vertices + v] != -1) {
        return RG_ERR_DUPLICATE_EDGE;
    }
    if (graph->n_edges >= graph->max_edges) {
        return RG_ERR_ALLOC;
    }

    /* Allocate nodes before touching any graph state to ensure atomic rollback on alloc failure */
    RgAdjNode *node_u = rg_malloc(sizeof(RgAdjNode));
    if (node_u == NULL) {
        return RG_ERR_ALLOC;
    }
    RgAdjNode *node_v = rg_malloc(sizeof(RgAdjNode));
    if (node_v == NULL) {
        rg_free(node_u);
        return RG_ERR_ALLOC;
    }

    int edge_id = graph->n_edges++;
    graph->edges[edge_id].u = u;
    graph->edges[edge_id].v = v;
    graph->edges[edge_id].active = true;

    /* Update adjacency matrix */
    graph->matrix[u * graph->n_vertices + v] = edge_id;
    graph->matrix[v * graph->n_vertices + u] = edge_id;

    /* Sorted insertion for u's list (ascending by neighbor ID) */
    node_u->neighbor = v;
    node_u->edge_id = edge_id;
    RgAdjNode **curr_u = &graph->adj_list[u];
    while (*curr_u != NULL && (*curr_u)->neighbor < v) {
        curr_u = &(*curr_u)->next;
    }
    node_u->next = *curr_u;
    *curr_u = node_u;

    /* Sorted insertion for v's list (ascending by neighbor ID) */
    node_v->neighbor = u;
    node_v->edge_id = edge_id;
    RgAdjNode **curr_v = &graph->adj_list[v];
    while (*curr_v != NULL && (*curr_v)->neighbor < u) {
        curr_v = &(*curr_v)->next;
    }
    node_v->next = *curr_v;
    *curr_v = node_v;

    if (out_edge_id != NULL) {
        *out_edge_id = edge_id;
    }
    return RG_OK;
}

RgStatus rg_disable_edge_by_id(RgGraph *graph, int edge_id) {
    if (graph == NULL || edge_id < 0 || edge_id >= graph->n_edges) {
        return RG_ERR_INVALID_PARAM;
    }
    if (!graph->edges[edge_id].active) {
        return RG_NOOP;
    }
    graph->edges[edge_id].active = false;
    return RG_OK;
}

RgStatus rg_enable_edge_by_id(RgGraph *graph, int edge_id) {
    if (graph == NULL || edge_id < 0 || edge_id >= graph->n_edges) {
        return RG_ERR_INVALID_PARAM;
    }
    if (graph->edges[edge_id].active) {
        return RG_NOOP;
    }
    graph->edges[edge_id].active = true;
    return RG_OK;
}

RgStatus rg_disable_edge_by_endpoints(RgGraph *graph, int u, int v) {
    if (graph == NULL || u < 0 || u >= graph->n_vertices || v < 0 || v >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    int edge_id = graph->matrix[u * graph->n_vertices + v];
    if (edge_id == -1) {
        return RG_ERR_NOT_FOUND;
    }
    return rg_disable_edge_by_id(graph, edge_id);
}

RgStatus rg_enable_edge_by_endpoints(RgGraph *graph, int u, int v) {
    if (graph == NULL || u < 0 || u >= graph->n_vertices || v < 0 || v >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    int edge_id = graph->matrix[u * graph->n_vertices + v];
    if (edge_id == -1) {
        return RG_ERR_NOT_FOUND;
    }
    return rg_enable_edge_by_id(graph, edge_id);
}

RgStatus rg_graph_reset(RgGraph *graph) {
    if (graph == NULL) {
        return RG_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < graph->n_edges; i++) {
        graph->edges[i].active = true;
    }
    return RG_OK;
}

int rg_get_edge_id(const RgGraph *graph, int u, int v) {
    if (graph == NULL || u < 0 || u >= graph->n_vertices || v < 0 || v >= graph->n_vertices) {
        return -1;
    }
    return graph->matrix[u * graph->n_vertices + v];
}

bool rg_is_edge_active(const RgGraph *graph, int edge_id) {
    if (graph == NULL || edge_id < 0 || edge_id >= graph->n_edges) {
        return false;
    }
    return graph->edges[edge_id].active;
}

const char *rg_status_string(RgStatus status) {
    switch (status) {
        case RG_OK: return "RG_OK: Success";
        case RG_NOOP: return "RG_NOOP: No-op (state unchanged)";
        case RG_NO_ROUTE: return "RG_NO_ROUTE: Unreachable destination";
        case RG_ERR_INVALID_PARAM: return "RG_ERR_INVALID_PARAM: Invalid parameter";
        case RG_ERR_SELF_LOOP: return "RG_ERR_SELF_LOOP: Self-loops not permitted";
        case RG_ERR_DUPLICATE_EDGE: return "RG_ERR_DUPLICATE_EDGE: Edge already exists";
        case RG_ERR_ALLOC: return "RG_ERR_ALLOC: Memory allocation failed";
        case RG_ERR_NOT_FOUND: return "RG_ERR_NOT_FOUND: Edge not found";
        default: return "RG_ERR_UNKNOWN: Unknown error";
    }
}
