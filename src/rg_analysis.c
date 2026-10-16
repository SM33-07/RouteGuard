#include "rg_analysis.h"
#include "rg_traverse.h"
#include "rg_alloc.h"
#include "rg_queue.h"
#include <string.h>

int rg_connected_components_matrix(const RgGraph *graph, int *out_component_ids) {
    if (graph == NULL || graph->n_vertices < 1) {
        return 0;
    }

    bool *visited = rg_calloc((size_t)graph->n_vertices, sizeof(bool));
    if (visited == NULL) {
        return -1;
    }

    RgQueue q;
    if (rg_queue_init(&q, graph->n_vertices) != RG_OK) {
        rg_free(visited);
        return -1;
    }

    int comp_count = 0;
    for (int i = 0; i < graph->n_vertices; i++) {
        if (!visited[i]) {
            visited[i] = true;
            if (out_component_ids != NULL) {
                out_component_ids[i] = comp_count;
            }
            rg_queue_enqueue(&q, i);

            while (!rg_queue_is_empty(&q)) {
                int u;
                rg_queue_dequeue(&q, &u);

                for (int v = 0; v < graph->n_vertices; v++) {
                    int edge_id = graph->matrix[u * graph->n_vertices + v];
                    if (edge_id != -1 && graph->edges[edge_id].active) {
                        if (!visited[v]) {
                            visited[v] = true;
                            if (out_component_ids != NULL) {
                                out_component_ids[v] = comp_count;
                            }
                            rg_queue_enqueue(&q, v);
                        }
                    }
                }
            }
            comp_count++;
        }
    }

    rg_queue_destroy(&q);
    rg_free(visited);
    return comp_count;
}

int rg_connected_components_list(const RgGraph *graph, int *out_component_ids) {
    if (graph == NULL || graph->n_vertices < 1) {
        return 0;
    }

    bool *visited = rg_calloc((size_t)graph->n_vertices, sizeof(bool));
    if (visited == NULL) {
        return -1;
    }

    RgQueue q;
    if (rg_queue_init(&q, graph->n_vertices) != RG_OK) {
        rg_free(visited);
        return -1;
    }

    int comp_count = 0;
    for (int i = 0; i < graph->n_vertices; i++) {
        if (!visited[i]) {
            visited[i] = true;
            if (out_component_ids != NULL) {
                out_component_ids[i] = comp_count;
            }
            rg_queue_enqueue(&q, i);

            while (!rg_queue_is_empty(&q)) {
                int u;
                rg_queue_dequeue(&q, &u);

                RgAdjNode *node = graph->adj_list[u];
                while (node != NULL) {
                    int edge_id = node->edge_id;
                    if (graph->edges[edge_id].active) {
                        int v = node->neighbor;
                        if (!visited[v]) {
                            visited[v] = true;
                            if (out_component_ids != NULL) {
                                out_component_ids[v] = comp_count;
                            }
                            rg_queue_enqueue(&q, v);
                        }
                    }
                    node = node->next;
                }
            }
            comp_count++;
        }
    }

    rg_queue_destroy(&q);
    rg_free(visited);
    return comp_count;
}

RgStatus rg_get_network_state(const RgGraph *graph, int src, int dst, bool use_matrix, RgNetworkState *out_state) {
    if (graph == NULL || out_state == NULL ||
        src < 0 || src >= graph->n_vertices || dst < 0 || dst >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }

    RgTraverseResult *res = NULL;
    RgStatus status = rg_traverse_result_create(graph->n_vertices, &res);
    if (status != RG_OK) {
        return status;
    }

    if (use_matrix) {
        status = bfs_matrix(graph, src, res);
    } else {
        status = bfs_list(graph, src, res);
    }
    if (status != RG_OK) {
        rg_traverse_result_free(res);
        return status;
    }

    out_state->reachable_src_count = res->order_len;
    out_state->component_count = use_matrix ? rg_connected_components_matrix(graph, NULL)
                                            : rg_connected_components_list(graph, NULL);

    if (res->visited[dst]) {
        out_state->route_found = true;
        out_state->route_hops = res->dist[dst];
    } else {
        out_state->route_found = false;
        out_state->route_hops = -1;
    }

    rg_traverse_result_free(res);
    return RG_OK;
}

RgStatus rg_failure_table_create(int n_edges, RgFailureTable **out_table) {
    if (out_table == NULL || n_edges < 0) {
        return RG_ERR_INVALID_PARAM;
    }
    RgFailureTable *t = rg_malloc(sizeof(RgFailureTable));
    if (t == NULL) {
        return RG_ERR_ALLOC;
    }
    t->n_rows = n_edges;
    t->rows = (n_edges > 0) ? rg_calloc((size_t)n_edges, sizeof(RgFailureRow)) : NULL;
    if (n_edges > 0 && t->rows == NULL) {
        rg_free(t);
        return RG_ERR_ALLOC;
    }
    *out_table = t;
    return RG_OK;
}

void rg_failure_table_free(RgFailureTable *table) {
    if (table != NULL) {
        rg_free(table->rows);
        rg_free(table);
    }
}

RgStatus rg_compute_failure_table(RgGraph *graph, int src, int dst, RgFailureTable *table) {
    if (graph == NULL || table == NULL || table->n_rows != graph->n_edges ||
        src < 0 || src >= graph->n_vertices || dst < 0 || dst >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }

    bool *orig_active = rg_malloc((size_t)graph->n_edges * sizeof(bool));
    if (orig_active == NULL) {
        return RG_ERR_ALLOC;
    }
    for (int i = 0; i < graph->n_edges; i++) {
        orig_active[i] = graph->edges[i].active;
    }

    RgTraverseResult *res = NULL;
    RgStatus status = rg_traverse_result_create(graph->n_vertices, &res);
    if (status != RG_OK) {
        rg_free(orig_active);
        return status;
    }

    /* Brute-force repeated BFS loop over all edges: O(E * (V + E)) */
    for (int e = 0; e < graph->n_edges; e++) {
        RgFailureRow *row = &table->rows[e];
        row->edge_id = e;
        row->u = graph->edges[e].u;
        row->v = graph->edges[e].v;

        /* Temporarily disable edge e */
        graph->edges[e].active = false;

        /* Run list BFS from source */
        bfs_list(graph, src, res);
        row->reachable = res->visited[dst];
        row->hop_count = row->reachable ? res->dist[dst] : -1;
        row->reachable_src_count = res->order_len;
        row->component_count = rg_connected_components_list(graph, NULL);

        /* Re-enable edge e to baseline state */
        graph->edges[e].active = orig_active[e];
    }

    /* Assert graph state is restored byte-identical to baseline */
    for (int i = 0; i < graph->n_edges; i++) {
        if (graph->edges[i].active != orig_active[i]) {
            /* Emergency restore if drift detected */
            graph->edges[i].active = orig_active[i];
        }
    }

    rg_traverse_result_free(res);
    rg_free(orig_active);
    return RG_OK;
}
