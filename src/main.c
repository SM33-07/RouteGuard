#include "rg_graph.h"
#include "rg_traverse.h"
#include "rg_analysis.h"
#include "rg_fixture.h"
#include "rg_svg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_impact_summary(const RgNetworkState *before, const RgNetworkState *after, int src, int dst) {
    printf("\n--- Impact Summary (Source: %d, Destination: %d) ---\n", src, dst);
    printf("Reachable vertices from %d : %d -> %d\n", src, before->reachable_src_count, after->reachable_src_count);
    printf("Total connected components : %d -> %d\n", before->component_count, after->component_count);
    if (before->route_found && after->route_found) {
        printf("Shortest route hops        : %d -> %d\n", before->route_hops, after->route_hops);
    } else if (before->route_found && !after->route_found) {
        printf("Shortest route hops        : %d -> NO ROUTE (disconnected)\n", before->route_hops);
    } else if (!before->route_found && after->route_found) {
        printf("Shortest route hops        : NO ROUTE -> %d\n", after->route_hops);
    } else {
        printf("Shortest route hops        : NO ROUTE -> NO ROUTE\n");
    }
    printf("----------------------------------------------------\n");
}

static void print_route(const RgRoute *route) {
    if (!route->found) {
        printf("Route: NO ROUTE (destination unreachable)\n");
        return;
    }
    printf("Route (%d hops): ", route->hop_count);
    for (int i = 0; i <= route->hop_count; i++) {
        printf("%d%s", route->path[i], (i == route->hop_count) ? "" : " -> ");
    }
    printf("\n");
}

static void print_failure_table(const RgFailureTable *table, int src, int dst) {
    printf("\n=== Single-Edge Failure Impact Table (Source: %d, Destination: %d) ===\n", src, dst);
    printf("Method: Brute-force repeated BFS, O(E*(V+E)) on adjacency lists\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("Edge ID | Endpoints | Reachable? | Hops | Reachable from %d | Components\n", src);
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < table->n_rows; i++) {
        const RgFailureRow *r = &table->rows[i];
        if (r->reachable) {
            printf("  e%-4d | (%2d, %2d)  |    YES     |  %2d  |        %2d         |     %2d\n",
                   r->edge_id, r->u, r->v, r->hop_count, r->reachable_src_count, r->component_count);
        } else {
            printf("  e%-4d | (%2d, %2d)  |     NO     |  --  |        %2d         |     %2d\n",
                   r->edge_id, r->u, r->v, r->reachable_src_count, r->component_count);
        }
    }
    printf("--------------------------------------------------------------------------------\n");
}

static int run_demo(void) {
    printf("======================================================================\n");
    printf("RouteGuard Deterministic Demonstration (Syllabus Scope: ISO C11)\n");
    printf("Educational Disclaimer: Synthetic campus network topology only.\n");
    printf("======================================================================\n");

    RgGraph *g = NULL;
    if (rg_fixture_f1_campus(&g) != RG_OK) {
        fprintf(stderr, "Failed to initialize fixture F1\n");
        return 1;
    }

    int src = 0;
    int dst = 10;
    RgRoute *route = NULL;
    rg_route_create(g->n_vertices, &route);

    /* 1. Baseline Route */
    printf("\n[Step 1] Baseline Route Evaluation: Source %d (Main Gate) -> Destination %d (Medical Annex)\n", src, dst);
    rg_find_route(g, src, dst, false, route, NULL);
    print_route(route);

    /* Export baseline SVG */
    rg_export_svg(g, route, NULL, "out/baseline.svg", "Baseline Route 0 -> 10 (8 hops)");

    /* 2. Disable e9 (6-7) */
    printf("\n[Step 2] Simulated Failure: Disabling critical walkway e9 (6, 7) [Hostel Junction - Sports Complex]\n");
    RgNetworkState before, after;
    rg_get_network_state(g, src, dst, false, &before);
    rg_disable_edge_by_id(g, 9);
    rg_get_network_state(g, src, dst, false, &after);
    print_impact_summary(&before, &after, src, dst);
    rg_find_route(g, src, dst, false, route, NULL);
    print_route(route);

    /* Compute unreached vertices from 0 */
    RgTraverseResult *bfs_e9 = NULL;
    rg_traverse_result_create(g->n_vertices, &bfs_e9);
    bfs_list(g, src, bfs_e9);
    bool unreached_e9[12];
    for (int i = 0; i < 12; i++) {
        unreached_e9[i] = !bfs_e9->visited[i];
    }
    rg_export_svg(g, route, unreached_e9, "out/e9_disabled.svg", "Simulated Failure: e9 Closed (Destination Unreachable)");
    rg_traverse_result_free(bfs_e9);

    /* 3. Re-enable e9 */
    printf("\n[Step 3] Restoring walkway e9 (6, 7)\n");
    rg_get_network_state(g, src, dst, false, &before);
    rg_enable_edge_by_id(g, 9);
    rg_get_network_state(g, src, dst, false, &after);
    print_impact_summary(&before, &after, src, dst);

    /* 4. Disable e5 (3-4) */
    printf("\n[Step 4] Simulated Failure: Disabling redundant walkway e5 (3, 4) [Central Plaza - Block A]\n");
    rg_get_network_state(g, src, dst, false, &before);
    rg_disable_edge_by_id(g, 5);
    rg_get_network_state(g, src, dst, false, &after);
    print_impact_summary(&before, &after, src, dst);
    rg_find_route(g, src, dst, false, route, NULL);
    print_route(route);
    rg_export_svg(g, route, NULL, "out/e5_disabled.svg", "Simulated Failure: e5 Closed (Rerouted via Block B, 8 hops)");

    /* 5. Reset Graph */
    printf("\n[Step 5] Resetting graph to baseline state\n");
    rg_graph_reset(g);

    /* 6. Representation Comparison */
    printf("\n[Step 6] Traversal Performance & Representation Comparison (F1 Campus Baseline)\n");
    RgTraverseResult *mat_bfs = NULL, *list_bfs = NULL;
    RgTraverseResult *mat_dfs = NULL, *list_dfs = NULL;
    rg_traverse_result_create(g->n_vertices, &mat_bfs);
    rg_traverse_result_create(g->n_vertices, &list_bfs);
    rg_traverse_result_create(g->n_vertices, &mat_dfs);
    rg_traverse_result_create(g->n_vertices, &list_dfs);

    bfs_matrix(g, src, mat_bfs);
    bfs_list(g, src, list_bfs);
    dfs_matrix(g, src, mat_dfs);
    dfs_list(g, src, list_dfs);

    printf("-------------------------------------------------------------------------------\n");
    printf("Algorithm | Representation | Processed | Slots Examined | Skips | Enq/Push | Deq/Pop\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("BFS       | Matrix (O(V^2))|    %2d     |      %3d       |  %2d   |    %2d    |   %2d\n",
           mat_bfs->metrics.vertices_processed, mat_bfs->metrics.neighbor_slots_examined,
           mat_bfs->metrics.edges_skipped_inactive, mat_bfs->metrics.queue_enqueues, mat_bfs->metrics.queue_dequeues);
    printf("BFS       | List   (O(V+E))|    %2d     |      %3d       |  %2d   |    %2d    |   %2d\n",
           list_bfs->metrics.vertices_processed, list_bfs->metrics.neighbor_slots_examined,
           list_bfs->metrics.edges_skipped_inactive, list_bfs->metrics.queue_enqueues, list_bfs->metrics.queue_dequeues);
    printf("DFS       | Matrix (O(V^2))|    %2d     |      %3d       |  %2d   |    %2d    |   %2d (stale: %d)\n",
           mat_dfs->metrics.vertices_processed, mat_dfs->metrics.neighbor_slots_examined,
           mat_dfs->metrics.edges_skipped_inactive, mat_dfs->metrics.stack_pushes, mat_dfs->metrics.stack_pops, mat_dfs->metrics.stale_pops);
    printf("DFS       | List   (O(V+E))|    %2d     |      %3d       |  %2d   |    %2d    |   %2d (stale: %d)\n",
           list_dfs->metrics.vertices_processed, list_dfs->metrics.neighbor_slots_examined,
           list_dfs->metrics.edges_skipped_inactive, list_dfs->metrics.stack_pushes, list_dfs->metrics.stack_pops, list_dfs->metrics.stale_pops);
    printf("-------------------------------------------------------------------------------\n");

    /* 7. Failure Table */
    RgFailureTable *ftable = NULL;
    rg_failure_table_create(g->n_edges, &ftable);
    rg_compute_failure_table(g, src, dst, ftable);
    print_failure_table(ftable, src, dst);

    /* Cleanup */
    rg_failure_table_free(ftable);
    rg_traverse_result_free(mat_bfs);
    rg_traverse_result_free(list_bfs);
    rg_traverse_result_free(mat_dfs);
    rg_traverse_result_free(list_dfs);
    rg_route_free(route);
    rg_graph_free(g);

    printf("\nDemo completed successfully. SVG figures generated in out/.\n");
    return 0;
}

static void print_menu(bool use_matrix) {
    printf("\n================ RouteGuard CLI ================\n");
    printf("Active Representation: %s\n", use_matrix ? "Adjacency MATRIX [O(V^2)]" : "Adjacency LIST [O(V+E)]");
    printf("1.  Display Graph Summary & Active Edges\n");
    printf("2.  Set Source and Destination Vertices\n");
    printf("3.  Run BFS Traversal\n");
    printf("4.  Run DFS Traversal\n");
    printf("5.  Find Shortest-Hop Route (BFS)\n");
    printf("6.  Check Reachability (DFS)\n");
    printf("7.  Count Connected Components\n");
    printf("8.  Disable an Edge (by ID)\n");
    printf("9.  Enable an Edge (by ID)\n");
    printf("10. Reset All Edges (Restore Baseline)\n");
    printf("11. Switch Representation (Matrix <-> List)\n");
    printf("12. Generate Single-Edge Failure Table\n");
    printf("13. Export Topology to SVG\n");
    printf("14. Display Complexity Analysis Notes\n");
    printf("0.  Quit\n");
    printf("Select an option [0-14]: ");
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        return run_demo();
    }

    RgGraph *g = NULL;
    if (rg_fixture_f1_campus(&g) != RG_OK) {
        fprintf(stderr, "Failed to load default campus fixture\n");
        return 1;
    }

    int src = 0;
    int dst = 10;
    bool use_matrix = false;
    char line_buf[128];

    printf("Welcome to RouteGuard (ISO C11 Network Connectivity Simulator)\n");
    printf("Educational Disclaimer: Synthetic campus network topology only.\n");

    while (1) {
        print_menu(use_matrix);
        if (fgets(line_buf, sizeof(line_buf), stdin) == NULL) {
            break;
        }

        char *endptr = NULL;
        long choice = strtol(line_buf, &endptr, 10);
        if (endptr == line_buf) {
            printf("Invalid input. Please enter a valid number.\n");
            continue;
        }

        if (choice == 0) {
            printf("Exiting RouteGuard. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1: {
                printf("\n--- Graph Overview ---\n");
                printf("Vertices (%d):\n", g->n_vertices);
                for (int i = 0; i < g->n_vertices; i++) {
                    printf("  %2d: %-18s (x=%4d, y=%4d)\n", i, g->labels[i], g->coords_x[i], g->coords_y[i]);
                }
                printf("Edges (%d):\n", g->n_edges);
                for (int i = 0; i < g->n_edges; i++) {
                    printf("  e%-2d: (%2d, %2d) [%s]\n", i, g->edges[i].u, g->edges[i].v,
                           g->edges[i].active ? "ACTIVE" : "CLOSED/DISABLED");
                }
                break;
            }
            case 2: {
                printf("Enter source vertex ID (0-%d): ", g->n_vertices - 1);
                if (fgets(line_buf, sizeof(line_buf), stdin) != NULL) {
                    long new_src = strtol(line_buf, &endptr, 10);
                    if (endptr != line_buf && new_src >= 0 && new_src < g->n_vertices) {
                        src = (int)new_src;
                    } else {
                        printf("Invalid source ID.\n");
                    }
                }
                printf("Enter destination vertex ID (0-%d): ", g->n_vertices - 1);
                if (fgets(line_buf, sizeof(line_buf), stdin) != NULL) {
                    long new_dst = strtol(line_buf, &endptr, 10);
                    if (endptr != line_buf && new_dst >= 0 && new_dst < g->n_vertices) {
                        dst = (int)new_dst;
                    } else {
                        printf("Invalid destination ID.\n");
                    }
                }
                printf("Source set to %d (%s), Destination set to %d (%s)\n",
                       src, g->labels[src], dst, g->labels[dst]);
                break;
            }
            case 3: {
                RgTraverseResult *res = NULL;
                rg_traverse_result_create(g->n_vertices, &res);
                if (use_matrix) {
                    bfs_matrix(g, src, res);
                } else {
                    bfs_list(g, src, res);
                }
                printf("\nBFS Visit Order from %d (%s): ", src, g->labels[src]);
                for (int i = 0; i < res->order_len; i++) {
                    printf("%d%s", res->order[i], (i == res->order_len - 1) ? "" : ", ");
                }
                printf("\nMetrics: Processed=%d, SlotsExamined=%d, Enqueues=%d, Dequeues=%d, InactiveSkipped=%d\n",
                       res->metrics.vertices_processed, res->metrics.neighbor_slots_examined,
                       res->metrics.queue_enqueues, res->metrics.queue_dequeues, res->metrics.edges_skipped_inactive);
                rg_traverse_result_free(res);
                break;
            }
            case 4: {
                RgTraverseResult *res = NULL;
                rg_traverse_result_create(g->n_vertices, &res);
                if (use_matrix) {
                    dfs_matrix(g, src, res);
                } else {
                    dfs_list(g, src, res);
                }
                printf("\nIterative DFS Visit Order from %d (%s): ", src, g->labels[src]);
                for (int i = 0; i < res->order_len; i++) {
                    printf("%d%s", res->order[i], (i == res->order_len - 1) ? "" : ", ");
                }
                printf("\nMetrics: Processed=%d, SlotsExamined=%d, Pushes=%d, Pops=%d, StalePops=%d, InactiveSkipped=%d\n",
                       res->metrics.vertices_processed, res->metrics.neighbor_slots_examined,
                       res->metrics.stack_pushes, res->metrics.stack_pops, res->metrics.stale_pops, res->metrics.edges_skipped_inactive);
                rg_traverse_result_free(res);
                break;
            }
            case 5: {
                RgRoute *rt = NULL;
                rg_route_create(g->n_vertices, &rt);
                RgMetrics met;
                RgStatus st = rg_find_route(g, src, dst, use_matrix, rt, &met);
                if (st == RG_OK) {
                    print_route(rt);
                } else if (st == RG_NO_ROUTE) {
                    printf("No active route exists between %d and %d.\n", src, dst);
                } else {
                    printf("Error finding route: %s\n", rg_status_string(st));
                }
                rg_route_free(rt);
                break;
            }
            case 6: {
                bool reach = rg_reachable(g, src, dst);
                printf("Vertex %d (%s) is %s from vertex %d (%s).\n",
                       dst, g->labels[dst], reach ? "REACHABLE" : "UNREACHABLE", src, g->labels[src]);
                break;
            }
            case 7: {
                int comps = use_matrix ? rg_connected_components_matrix(g, NULL)
                                       : rg_connected_components_list(g, NULL);
                printf("Total connected components in active graph: %d\n", comps);
                break;
            }
            case 8: {
                printf("Enter edge ID to disable (0-%d): ", g->n_edges - 1);
                if (fgets(line_buf, sizeof(line_buf), stdin) != NULL) {
                    long eid = strtol(line_buf, &endptr, 10);
                    if (endptr != line_buf && eid >= 0 && eid < g->n_edges) {
                        RgNetworkState before_state, after_state;
                        rg_get_network_state(g, src, dst, use_matrix, &before_state);
                        RgStatus st = rg_disable_edge_by_id(g, (int)eid);
                        rg_get_network_state(g, src, dst, use_matrix, &after_state);
                        printf("Operation result: %s\n", rg_status_string(st));
                        print_impact_summary(&before_state, &after_state, src, dst);
                    } else {
                        printf("Invalid edge ID.\n");
                    }
                }
                break;
            }
            case 9: {
                printf("Enter edge ID to enable (0-%d): ", g->n_edges - 1);
                if (fgets(line_buf, sizeof(line_buf), stdin) != NULL) {
                    long eid = strtol(line_buf, &endptr, 10);
                    if (endptr != line_buf && eid >= 0 && eid < g->n_edges) {
                        RgNetworkState before_state, after_state;
                        rg_get_network_state(g, src, dst, use_matrix, &before_state);
                        RgStatus st = rg_enable_edge_by_id(g, (int)eid);
                        rg_get_network_state(g, src, dst, use_matrix, &after_state);
                        printf("Operation result: %s\n", rg_status_string(st));
                        print_impact_summary(&before_state, &after_state, src, dst);
                    } else {
                        printf("Invalid edge ID.\n");
                    }
                }
                break;
            }
            case 10: {
                RgNetworkState before_state, after_state;
                rg_get_network_state(g, src, dst, use_matrix, &before_state);
                rg_graph_reset(g);
                rg_get_network_state(g, src, dst, use_matrix, &after_state);
                printf("All edges reactivated (baseline restored).\n");
                print_impact_summary(&before_state, &after_state, src, dst);
                break;
            }
            case 11: {
                use_matrix = !use_matrix;
                printf("Switched active representation to: %s\n",
                       use_matrix ? "Adjacency MATRIX [O(V^2)]" : "Adjacency LIST [O(V+E)]");
                break;
            }
            case 12: {
                RgFailureTable *ft = NULL;
                rg_failure_table_create(g->n_edges, &ft);
                rg_compute_failure_table(g, src, dst, ft);
                print_failure_table(ft, src, dst);
                rg_failure_table_free(ft);
                break;
            }
            case 13: {
                printf("Enter output SVG filename [default: out/topology.svg]: ");
                if (fgets(line_buf, sizeof(line_buf), stdin) != NULL) {
                    line_buf[strcspn(line_buf, "\r\n")] = '\0';
                    const char *target = (strlen(line_buf) > 0) ? line_buf : "out/topology.svg";
                    RgRoute *rt = NULL;
                    rg_route_create(g->n_vertices, &rt);
                    rg_find_route(g, src, dst, use_matrix, rt, NULL);
                    RgStatus st = rg_export_svg(g, rt, NULL, target, "Interactive Topology Export");
                    if (st == RG_OK) {
                        printf("Exported SVG successfully to %s\n", target);
                    } else {
                        printf("Failed to export SVG: %s\n", rg_status_string(st));
                    }
                    rg_route_free(rt);
                }
                break;
            }
            case 14: {
                printf("\n=== Representation Complexity Analysis ===\n");
                printf("1. Adjacency Matrix:\n");
                printf("   - Space: O(V^2). Always allocates V*V slots (144 slots on F1).\n");
                printf("   - Traversal Time: O(V^2). Inspects all V columns for each processed vertex.\n");
                printf("   - Edge Toggle: O(1) direct array flag update.\n");
                printf("2. Adjacency List:\n");
                printf("   - Space: O(V + E). Allocates V head pointers + 2E nodes (12 + 26 = 38 on F1).\n");
                printf("   - Traversal Time: O(V + E). Inspects only incident neighbor nodes (26 slots on F1).\n");
                printf("   - Edge Toggle: O(1) shared canonical active flag.\n");
                printf("Empirical difference on F1: Matrix scans 144 slots vs List scans 26 slots (5.54x reduction).\n");
                break;
            }
            default:
                printf("Option not recognized. Please choose between 0 and 14.\n");
                break;
        }
    }

    rg_graph_free(g);
    return 0;
}
