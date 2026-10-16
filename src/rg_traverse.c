#include "rg_traverse.h"
#include "rg_queue.h"
#include "rg_stack.h"
#include "rg_alloc.h"
#include <string.h>

RgStatus rg_traverse_result_create(int n_vertices, RgTraverseResult **out_res) {
    if (out_res == NULL || n_vertices < 1) {
        return RG_ERR_INVALID_PARAM;
    }
    RgTraverseResult *r = rg_malloc(sizeof(RgTraverseResult));
    if (r == NULL) {
        return RG_ERR_ALLOC;
    }
    r->order = rg_malloc((size_t)n_vertices * sizeof(int));
    r->visited = rg_calloc((size_t)n_vertices, sizeof(bool));
    r->dist = rg_malloc((size_t)n_vertices * sizeof(int));
    r->pred = rg_malloc((size_t)n_vertices * sizeof(int));

    if (r->order == NULL || r->visited == NULL || r->dist == NULL || r->pred == NULL) {
        rg_traverse_result_free(r);
        return RG_ERR_ALLOC;
    }

    r->order_len = 0;
    memset(&r->metrics, 0, sizeof(RgMetrics));
    *out_res = r;
    return RG_OK;
}

void rg_traverse_result_free(RgTraverseResult *res) {
    if (res != NULL) {
        rg_free(res->pred);
        rg_free(res->dist);
        rg_free(res->visited);
        rg_free(res->order);
        rg_free(res);
    }
}

RgStatus rg_route_create(int n_vertices, RgRoute **out_route) {
    if (out_route == NULL || n_vertices < 1) {
        return RG_ERR_INVALID_PARAM;
    }
    RgRoute *rt = rg_malloc(sizeof(RgRoute));
    if (rt == NULL) {
        return RG_ERR_ALLOC;
    }
    rt->path = rg_malloc((size_t)n_vertices * sizeof(int));
    if (rt->path == NULL) {
        rg_free(rt);
        return RG_ERR_ALLOC;
    }
    rt->hop_count = -1;
    rt->found = false;
    *out_route = rt;
    return RG_OK;
}

void rg_route_free(RgRoute *route) {
    if (route != NULL) {
        rg_free(route->path);
        rg_free(route);
    }
}

static void reset_traverse_arrays(const RgGraph *graph, RgTraverseResult *res) {
    res->order_len = 0;
    memset(&res->metrics, 0, sizeof(RgMetrics));
    for (int i = 0; i < graph->n_vertices; i++) {
        res->visited[i] = false;
        res->dist[i] = -1;
        res->pred[i] = -1;
    }
}

RgStatus bfs_matrix(const RgGraph *graph, int src, RgTraverseResult *res) {
    if (graph == NULL || res == NULL || src < 0 || src >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    reset_traverse_arrays(graph, res);

    RgQueue q;
    RgStatus status = rg_queue_init(&q, graph->n_vertices);
    if (status != RG_OK) {
        return status;
    }

    /* Mark visited ON ENQUEUE */
    res->visited[src] = true;
    res->dist[src] = 0;
    res->pred[src] = -1;
    rg_queue_enqueue(&q, src);
    res->metrics.queue_enqueues++;

    while (!rg_queue_is_empty(&q)) {
        int u;
        rg_queue_dequeue(&q, &u);
        res->metrics.queue_dequeues++;
        res->metrics.vertices_processed++;
        res->order[res->order_len++] = u;

        /* Scan matrix columns 0..n-1 in ascending order */
        for (int v = 0; v < graph->n_vertices; v++) {
            res->metrics.neighbor_slots_examined++;
            int edge_id = graph->matrix[u * graph->n_vertices + v];
            if (edge_id == -1) {
                continue;
            }
            if (!graph->edges[edge_id].active) {
                res->metrics.edges_skipped_inactive++;
                continue;
            }
            if (!res->visited[v]) {
                res->visited[v] = true;
                res->dist[v] = res->dist[u] + 1;
                res->pred[v] = u;
                rg_queue_enqueue(&q, v);
                res->metrics.queue_enqueues++;
            }
        }
    }

    rg_queue_destroy(&q);
    return RG_OK;
}

RgStatus bfs_list(const RgGraph *graph, int src, RgTraverseResult *res) {
    if (graph == NULL || res == NULL || src < 0 || src >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    reset_traverse_arrays(graph, res);

    RgQueue q;
    RgStatus status = rg_queue_init(&q, graph->n_vertices);
    if (status != RG_OK) {
        return status;
    }

    /* Mark visited ON ENQUEUE */
    res->visited[src] = true;
    res->dist[src] = 0;
    res->pred[src] = -1;
    rg_queue_enqueue(&q, src);
    res->metrics.queue_enqueues++;

    while (!rg_queue_is_empty(&q)) {
        int u;
        rg_queue_dequeue(&q, &u);
        res->metrics.queue_dequeues++;
        res->metrics.vertices_processed++;
        res->order[res->order_len++] = u;

        /* Inspect stored nodes in singly linked list (already sorted ascending) */
        RgAdjNode *node = graph->adj_list[u];
        while (node != NULL) {
            res->metrics.neighbor_slots_examined++;
            int edge_id = node->edge_id;
            if (!graph->edges[edge_id].active) {
                res->metrics.edges_skipped_inactive++;
                node = node->next;
                continue;
            }
            int v = node->neighbor;
            if (!res->visited[v]) {
                res->visited[v] = true;
                res->dist[v] = res->dist[u] + 1;
                res->pred[v] = u;
                rg_queue_enqueue(&q, v);
                res->metrics.queue_enqueues++;
            }
            node = node->next;
        }
    }

    rg_queue_destroy(&q);
    return RG_OK;
}

RgStatus dfs_matrix(const RgGraph *graph, int src, RgTraverseResult *res) {
    if (graph == NULL || res == NULL || src < 0 || src >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    reset_traverse_arrays(graph, res);

    int stack_cap = 2 * graph->n_edges + 1;
    if (stack_cap < 2) {
        stack_cap = 2;
    }
    RgStack s;
    RgStatus status = rg_stack_init(&s, stack_cap);
    if (status != RG_OK) {
        return status;
    }

    rg_stack_push(&s, src);
    res->metrics.stack_pushes++;

    while (!rg_stack_is_empty(&s)) {
        int u;
        rg_stack_pop(&s, &u);
        res->metrics.stack_pops++;

        /* Mark visited ON POP */
        if (res->visited[u]) {
            res->metrics.stale_pops++;
            continue;
        }
        res->visited[u] = true;
        res->metrics.vertices_processed++;
        res->order[res->order_len++] = u;

        /* Push unvisited active neighbors in DESCENDING ID order */
        for (int v = graph->n_vertices - 1; v >= 0; v--) {
            res->metrics.neighbor_slots_examined++;
            int edge_id = graph->matrix[u * graph->n_vertices + v];
            if (edge_id == -1) {
                continue;
            }
            if (!graph->edges[edge_id].active) {
                res->metrics.edges_skipped_inactive++;
                continue;
            }
            if (!res->visited[v]) {
                rg_stack_push(&s, v);
                res->metrics.stack_pushes++;
            }
        }
    }

    rg_stack_destroy(&s);
    return RG_OK;
}

RgStatus dfs_list(const RgGraph *graph, int src, RgTraverseResult *res) {
    if (graph == NULL || res == NULL || src < 0 || src >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    reset_traverse_arrays(graph, res);

    int stack_cap = 2 * graph->n_edges + 1;
    if (stack_cap < 2) {
        stack_cap = 2;
    }
    RgStack s;
    RgStatus status = rg_stack_init(&s, stack_cap);
    if (status != RG_OK) {
        return status;
    }

    rg_stack_push(&s, src);
    res->metrics.stack_pushes++;

    int *neighbor_buf = rg_malloc((size_t)graph->n_vertices * sizeof(int));
    if (neighbor_buf == NULL) {
        rg_stack_destroy(&s);
        return RG_ERR_ALLOC;
    }

    while (!rg_stack_is_empty(&s)) {
        int u;
        rg_stack_pop(&s, &u);
        res->metrics.stack_pops++;

        /* Mark visited ON POP */
        if (res->visited[u]) {
            res->metrics.stale_pops++;
            continue;
        }
        res->visited[u] = true;
        res->metrics.vertices_processed++;
        res->order[res->order_len++] = u;

        /* Collect active unvisited neighbors from ascending list */
        int count = 0;
        RgAdjNode *node = graph->adj_list[u];
        while (node != NULL) {
            res->metrics.neighbor_slots_examined++;
            int edge_id = node->edge_id;
            if (!graph->edges[edge_id].active) {
                res->metrics.edges_skipped_inactive++;
                node = node->next;
                continue;
            }
            int v = node->neighbor;
            if (!res->visited[v]) {
                neighbor_buf[count++] = v;
            }
            node = node->next;
        }

        /* Push in DESCENDING ID order so smallest ID is popped first */
        for (int i = count - 1; i >= 0; i--) {
            rg_stack_push(&s, neighbor_buf[i]);
            res->metrics.stack_pushes++;
        }
    }

    rg_free(neighbor_buf);
    rg_stack_destroy(&s);
    return RG_OK;
}

static void dfs_rec_helper(const RgGraph *graph, int u, RgTraverseResult *res) {
    res->visited[u] = true;
    res->order[res->order_len++] = u;
    res->metrics.vertices_processed++;

    RgAdjNode *node = graph->adj_list[u];
    while (node != NULL) {
        res->metrics.neighbor_slots_examined++;
        int edge_id = node->edge_id;
        if (!graph->edges[edge_id].active) {
            res->metrics.edges_skipped_inactive++;
            node = node->next;
            continue;
        }
        int v = node->neighbor;
        if (!res->visited[v]) {
            dfs_rec_helper(graph, v, res);
        }
        node = node->next;
    }
}

RgStatus dfs_list_recursive(const RgGraph *graph, int src, RgTraverseResult *res) {
    if (graph == NULL || res == NULL || src < 0 || src >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }
    reset_traverse_arrays(graph, res);
    dfs_rec_helper(graph, src, res);
    return RG_OK;
}

RgStatus rg_find_route(const RgGraph *graph, int src, int dst, bool use_matrix, RgRoute *out_route, RgMetrics *out_metrics) {
    if (graph == NULL || out_route == NULL ||
        src < 0 || src >= graph->n_vertices || dst < 0 || dst >= graph->n_vertices) {
        return RG_ERR_INVALID_PARAM;
    }

    RgTraverseResult *bfs_res = NULL;
    RgStatus status = rg_traverse_result_create(graph->n_vertices, &bfs_res);
    if (status != RG_OK) {
        return status;
    }

    if (use_matrix) {
        status = bfs_matrix(graph, src, bfs_res);
    } else {
        status = bfs_list(graph, src, bfs_res);
    }
    if (status != RG_OK) {
        rg_traverse_result_free(bfs_res);
        return status;
    }

    if (out_metrics != NULL) {
        *out_metrics = bfs_res->metrics;
    }

    if (!bfs_res->visited[dst]) {
        out_route->found = false;
        out_route->hop_count = -1;
        rg_traverse_result_free(bfs_res);
        return RG_NO_ROUTE;
    }

    if (src == dst) {
        out_route->found = true;
        out_route->hop_count = 0;
        out_route->path[0] = src;
        rg_traverse_result_free(bfs_res);
        return RG_OK;
    }

    /* Trace predecessor pointers from dst to src */
    int temp_len = 0;
    int curr = dst;
    while (curr != -1) {
        out_route->path[temp_len++] = curr;
        if (curr == src) {
            break;
        }
        curr = bfs_res->pred[curr];
    }

    /* Reverse path in place */
    for (int i = 0; i < temp_len / 2; i++) {
        int swap = out_route->path[i];
        out_route->path[i] = out_route->path[temp_len - 1 - i];
        out_route->path[temp_len - 1 - i] = swap;
    }

    out_route->found = true;
    out_route->hop_count = temp_len - 1;
    rg_traverse_result_free(bfs_res);
    return RG_OK;
}

bool rg_reachable(const RgGraph *graph, int src, int dst) {
    if (graph == NULL || src < 0 || src >= graph->n_vertices || dst < 0 || dst >= graph->n_vertices) {
        return false;
    }
    RgTraverseResult *res = NULL;
    if (rg_traverse_result_create(graph->n_vertices, &res) != RG_OK) {
        return false;
    }
    dfs_list(graph, src, res);
    bool reach = res->visited[dst];
    rg_traverse_result_free(res);
    return reach;
}
