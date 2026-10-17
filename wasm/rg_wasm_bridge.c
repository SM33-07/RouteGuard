#include "rg_wasm_bridge.h"
#include "rg_alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static RgGraph *g_wasm_graph = NULL;

#define JSON_BUF_SIZE 65536
#define SVG_BUF_SIZE 131072

static char s_json_buffer[JSON_BUF_SIZE];
static char s_svg_buffer[SVG_BUF_SIZE];

static int s_active_fixture = 0; /* 0 = F1, 1 = MUJ Campus */

static void ensure_graph_initialized(void) {
    if (g_wasm_graph == NULL) {
        if (s_active_fixture == 1) {
            rg_fixture_campus_muj(&g_wasm_graph);
        } else {
            rg_fixture_f1_campus(&g_wasm_graph);
        }
    }
}

int rg_wasm_init(void) {
    return rg_wasm_init_fixture(s_active_fixture);
}

int rg_wasm_init_fixture(int fixture_id) {
    if (g_wasm_graph != NULL) {
        rg_graph_free(g_wasm_graph);
        g_wasm_graph = NULL;
    }
    s_active_fixture = (fixture_id == 1) ? 1 : 0;
    RgStatus st;
    if (s_active_fixture == 1) {
        st = rg_fixture_campus_muj(&g_wasm_graph);
    } else {
        st = rg_fixture_f1_campus(&g_wasm_graph);
    }
    return (st == RG_OK) ? 0 : (int)st;
}

int rg_wasm_get_active_fixture(void) {
    return s_active_fixture;
}

int rg_wasm_reset(void) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL) {
        return -1;
    }
    RgStatus st = rg_graph_reset(g_wasm_graph);
    return (st == RG_OK) ? 0 : (int)st;
}

const char *rg_wasm_get_graph_json(void) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Graph initialization failed\"}");
        return s_json_buffer;
    }

    const char *fixture_name = (s_active_fixture == 1) ? "MUJ Campus Demo" : "F1 Reference Graph";
    int offset = snprintf(s_json_buffer, JSON_BUF_SIZE,
                          "{\"fixture_id\":%d,\"fixture_name\":\"%s\",\"n_vertices\":%d,\"n_edges\":%d,\"vertices\":[",
                          s_active_fixture, fixture_name, g_wasm_graph->n_vertices, g_wasm_graph->n_edges);

    for (int i = 0; i < g_wasm_graph->n_vertices; i++) {
        const char *comma = (i < g_wasm_graph->n_vertices - 1) ? "," : "";
        const char *full_desc = (s_active_fixture == 1)
                                ? rg_fixture_campus_venue_description(i)
                                : g_wasm_graph->labels[i];
        offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset,
                           "{\"id\":%d,\"label\":\"%s\",\"full_name\":\"%s\",\"x\":%d,\"y\":%d}%s",
                           i, g_wasm_graph->labels[i], full_desc,
                           g_wasm_graph->coords_x[i], g_wasm_graph->coords_y[i],
                           comma);
    }

    offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "],\"edges\":[");

    for (int e = 0; e < g_wasm_graph->n_edges; e++) {
        const char *comma = (e < g_wasm_graph->n_edges - 1) ? "," : "";
        offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset,
                           "{\"id\":%d,\"u\":%d,\"v\":%d,\"active\":%s}%s",
                           e, g_wasm_graph->edges[e].u, g_wasm_graph->edges[e].v,
                           g_wasm_graph->edges[e].active ? "true" : "false",
                           comma);
    }

    snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "]}");
    return s_json_buffer;
}


int rg_wasm_set_edge_active(int edge_id, int active) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL) {
        return -1;
    }
    RgStatus st;
    if (active) {
        st = rg_enable_edge_by_id(g_wasm_graph, edge_id);
    } else {
        st = rg_disable_edge_by_id(g_wasm_graph, edge_id);
    }
    return (st == RG_OK || st == RG_NOOP) ? 0 : (int)st;
}

static void format_metrics_json(char *buf, size_t buf_size, const RgMetrics *m) {
    snprintf(buf, buf_size,
             "{\"vertices_processed\":%d,"
             "\"neighbor_slots_examined\":%d,"
             "\"edges_skipped_inactive\":%d,"
             "\"queue_enqueues\":%d,"
             "\"queue_dequeues\":%d,"
             "\"stack_pushes\":%d,"
             "\"stack_pops\":%d,"
             "\"stale_pops\":%d,"
             "\"elapsed_ms\":%.4f}",
             m->vertices_processed,
             m->neighbor_slots_examined,
             m->edges_skipped_inactive,
             m->queue_enqueues,
             m->queue_dequeues,
             m->stack_pushes,
             m->stack_pops,
             m->stale_pops,
             m->elapsed_ms);
}

const char *rg_wasm_run_bfs(int src, int dst, int use_matrix) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL || src < 0 || src >= g_wasm_graph->n_vertices ||
        dst < 0 || dst >= g_wasm_graph->n_vertices) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Invalid source or destination vertex\"}");
        return s_json_buffer;
    }

    RgTraverseResult *res = NULL;
    RgStatus st = rg_traverse_result_create(g_wasm_graph->n_vertices, &res);
    if (st != RG_OK) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Memory allocation failure\"}");
        return s_json_buffer;
    }

    if (use_matrix) {
        bfs_matrix(g_wasm_graph, src, res);
    } else {
        bfs_list(g_wasm_graph, src, res);
    }

    RgRoute *route = NULL;
    rg_route_create(g_wasm_graph->n_vertices, &route);
    RgMetrics route_metrics;
    rg_find_route(g_wasm_graph, src, dst, use_matrix ? true : false, route, &route_metrics);

    char metrics_buf[512];
    format_metrics_json(metrics_buf, sizeof(metrics_buf), &res->metrics);

    int offset = snprintf(s_json_buffer, JSON_BUF_SIZE,
                          "{\"algorithm\":\"BFS\","
                          "\"representation\":\"%s\","
                          "\"src\":%d,"
                          "\"dst\":%d,"
                          "\"reachable\":%s,"
                          "\"dist\":%d,"
                          "\"found\":%s,"
                          "\"hop_count\":%d,"
                          "\"order_len\":%d,"
                          "\"order\":[",
                          use_matrix ? "Adjacency Matrix" : "Adjacency List",
                          src, dst,
                          (dst < g_wasm_graph->n_vertices && res->visited[dst]) ? "true" : "false",
                          res->dist[dst],
                          route->found ? "true" : "false",
                          route->hop_count,
                          res->order_len);

    for (int i = 0; i < res->order_len; i++) {
        const char *comma = (i < res->order_len - 1) ? "," : "";
        offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "%d%s", res->order[i], comma);
    }

    offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "],\"route\":[");

    if (route->found) {
        for (int i = 0; i <= route->hop_count; i++) {
            const char *comma = (i < route->hop_count) ? "," : "";
            offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "%d%s", route->path[i], comma);
        }
    }

    snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset,
             "],\"metrics\":%s}", metrics_buf);

    rg_traverse_result_free(res);
    rg_route_free(route);
    return s_json_buffer;
}

const char *rg_wasm_run_dfs(int src, int dst, int use_matrix) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL || src < 0 || src >= g_wasm_graph->n_vertices ||
        dst < 0 || dst >= g_wasm_graph->n_vertices) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Invalid source or destination vertex\"}");
        return s_json_buffer;
    }

    RgTraverseResult *res = NULL;
    RgStatus st = rg_traverse_result_create(g_wasm_graph->n_vertices, &res);
    if (st != RG_OK) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Memory allocation failure\"}");
        return s_json_buffer;
    }

    if (use_matrix) {
        dfs_matrix(g_wasm_graph, src, res);
    } else {
        dfs_list(g_wasm_graph, src, res);
    }

    char metrics_buf[512];
    format_metrics_json(metrics_buf, sizeof(metrics_buf), &res->metrics);

    int offset = snprintf(s_json_buffer, JSON_BUF_SIZE,
                          "{\"algorithm\":\"DFS\","
                          "\"representation\":\"%s\","
                          "\"src\":%d,"
                          "\"dst\":%d,"
                          "\"reachable\":%s,"
                          "\"order_len\":%d,"
                          "\"order\":[",
                          use_matrix ? "Adjacency Matrix" : "Adjacency List",
                          src, dst,
                          (dst < g_wasm_graph->n_vertices && res->visited[dst]) ? "true" : "false",
                          res->order_len);

    for (int i = 0; i < res->order_len; i++) {
        const char *comma = (i < res->order_len - 1) ? "," : "";
        offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "%d%s", res->order[i], comma);
    }

    snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset,
             "],\"metrics\":%s}", metrics_buf);

    rg_traverse_result_free(res);
    return s_json_buffer;
}

const char *rg_wasm_find_route(int src, int dst, int use_matrix) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL || src < 0 || src >= g_wasm_graph->n_vertices ||
        dst < 0 || dst >= g_wasm_graph->n_vertices) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Invalid source or destination vertex\"}");
        return s_json_buffer;
    }

    RgRoute *route = NULL;
    rg_route_create(g_wasm_graph->n_vertices, &route);
    RgMetrics metrics;
    rg_find_route(g_wasm_graph, src, dst, use_matrix ? true : false, route, &metrics);

    char metrics_buf[512];
    format_metrics_json(metrics_buf, sizeof(metrics_buf), &metrics);

    int offset = snprintf(s_json_buffer, JSON_BUF_SIZE,
                          "{\"src\":%d,"
                          "\"dst\":%d,"
                          "\"found\":%s,"
                          "\"hop_count\":%d,"
                          "\"path\":[",
                          src, dst,
                          route->found ? "true" : "false",
                          route->hop_count);

    if (route->found) {
        for (int i = 0; i <= route->hop_count; i++) {
            const char *comma = (i < route->hop_count) ? "," : "";
            offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "%d%s", route->path[i], comma);
        }
    }

    snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset,
             "],\"metrics\":%s}", metrics_buf);

    rg_route_free(route);
    return s_json_buffer;
}

const char *rg_wasm_get_components(int use_matrix) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Graph not initialized\"}");
        return s_json_buffer;
    }

    int *comp_ids = (int *)rg_malloc((size_t)g_wasm_graph->n_vertices * sizeof(int));
    if (comp_ids == NULL) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Memory allocation error\"}");
        return s_json_buffer;
    }

    int count = 0;
    if (use_matrix) {
        count = rg_connected_components_matrix(g_wasm_graph, comp_ids);
    } else {
        count = rg_connected_components_list(g_wasm_graph, comp_ids);
    }

    int offset = snprintf(s_json_buffer, JSON_BUF_SIZE,
                          "{\"component_count\":%d,\"components\":[", count);

    for (int i = 0; i < g_wasm_graph->n_vertices; i++) {
        const char *comma = (i < g_wasm_graph->n_vertices - 1) ? "," : "";
        offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset,
                           "{\"vertex\":%d,\"component_id\":%d}%s", i, comp_ids[i], comma);
    }

    snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "]}");
    rg_free(comp_ids);
    return s_json_buffer;
}

const char *rg_wasm_get_network_state(int src, int dst, int use_matrix) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Graph not initialized\"}");
        return s_json_buffer;
    }

    RgNetworkState state;
    RgStatus st = rg_get_network_state(g_wasm_graph, src, dst, use_matrix ? true : false, &state);
    if (st != RG_OK) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Failed to get network state\"}");
        return s_json_buffer;
    }

    snprintf(s_json_buffer, JSON_BUF_SIZE,
             "{\"reachable_src_count\":%d,"
             "\"component_count\":%d,"
             "\"route_hops\":%d,"
             "\"route_found\":%s}",
             state.reachable_src_count,
             state.component_count,
             state.route_hops,
             state.route_found ? "true" : "false");

    return s_json_buffer;
}

const char *rg_wasm_get_failure_table(int src, int dst) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL || src < 0 || src >= g_wasm_graph->n_vertices ||
        dst < 0 || dst >= g_wasm_graph->n_vertices) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Invalid parameters\"}");
        return s_json_buffer;
    }

    RgFailureTable *table = NULL;
    RgStatus st = rg_failure_table_create(g_wasm_graph->n_edges, &table);
    if (st != RG_OK) {
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Memory allocation failure\"}");
        return s_json_buffer;
    }

    st = rg_compute_failure_table(g_wasm_graph, src, dst, table);
    if (st != RG_OK) {
        rg_failure_table_free(table);
        snprintf(s_json_buffer, JSON_BUF_SIZE, "{\"error\":\"Failure table calculation failed\"}");
        return s_json_buffer;
    }

    int offset = snprintf(s_json_buffer, JSON_BUF_SIZE,
                          "{\"src\":%d,\"dst\":%d,\"n_rows\":%d,\"rows\":[",
                          src, dst, table->n_rows);

    for (int i = 0; i < table->n_rows; i++) {
        const char *comma = (i < table->n_rows - 1) ? "," : "";
        offset += snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset,
                           "{\"edge_id\":%d,\"u\":%d,\"v\":%d,\"reachable\":%s,"
                           "\"hop_count\":%d,\"reachable_src_count\":%d,\"component_count\":%d}%s",
                           table->rows[i].edge_id,
                           table->rows[i].u,
                           table->rows[i].v,
                           table->rows[i].reachable ? "true" : "false",
                           table->rows[i].hop_count,
                           table->rows[i].reachable_src_count,
                           table->rows[i].component_count,
                           comma);
    }

    snprintf(s_json_buffer + offset, JSON_BUF_SIZE - offset, "]}");
    rg_failure_table_free(table);
    return s_json_buffer;
}

const char *rg_wasm_export_svg(int src, int dst, int use_matrix) {
    ensure_graph_initialized();
    if (g_wasm_graph == NULL) {
        snprintf(s_svg_buffer, SVG_BUF_SIZE, "<!-- Graph not initialized -->");
        return s_svg_buffer;
    }

    RgTraverseResult *res = NULL;
    rg_traverse_result_create(g_wasm_graph->n_vertices, &res);
    if (use_matrix) {
        bfs_matrix(g_wasm_graph, src, res);
    } else {
        bfs_list(g_wasm_graph, src, res);
    }

    RgRoute *route = NULL;
    rg_route_create(g_wasm_graph->n_vertices, &route);
    RgMetrics metrics;
    rg_find_route(g_wasm_graph, src, dst, use_matrix ? true : false, route, &metrics);

    bool *unreached = (bool *)rg_malloc((size_t)g_wasm_graph->n_vertices * sizeof(bool));
    if (unreached != NULL) {
        for (int i = 0; i < g_wasm_graph->n_vertices; i++) {
            unreached[i] = !res->visited[i];
        }
    }

    const char *tmp_path = "/tmp_rg_export.svg";
    char subtitle[128];
    snprintf(subtitle, sizeof(subtitle), "Route: %d (%s) -> %d (%s) | Representation: %s",
             src, g_wasm_graph->labels[src], dst, g_wasm_graph->labels[dst],
             use_matrix ? "Adjacency Matrix" : "Adjacency List");

    rg_export_svg(g_wasm_graph, route, unreached, tmp_path, subtitle);

    FILE *f = fopen(tmp_path, "r");
    if (f != NULL) {
        size_t n = fread(s_svg_buffer, 1, SVG_BUF_SIZE - 1, f);
        s_svg_buffer[n] = '\0';
        fclose(f);
    } else {
        snprintf(s_svg_buffer, SVG_BUF_SIZE, "<!-- Failed to read generated SVG -->");
    }

    if (unreached != NULL) {
        rg_free(unreached);
    }
    rg_route_free(route);
    rg_traverse_result_free(res);

    return s_svg_buffer;
}
