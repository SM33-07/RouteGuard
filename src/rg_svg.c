#include "rg_svg.h"
#include <stdio.h>
#include <string.h>

RgStatus rg_export_svg(const RgGraph *graph, const RgRoute *route, const bool *unreached, const char *filename, const char *subtitle) {
    if (graph == NULL || filename == NULL) {
        return RG_ERR_INVALID_PARAM;
    }

    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        return RG_ERR_ALLOC;
    }

    /* SVG Header - strict XML compliant */
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg width=\"1200\" height=\"500\" viewBox=\"0 0 1200 500\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    fprintf(f, "  <rect width=\"100%%\" height=\"100%%\" fill=\"#f8f9fa\" />\n");

    /* Educational Disclaimer and Title */
    fprintf(f, "  <text x=\"30\" y=\"35\" font-family=\"Arial, sans-serif\" font-size=\"20\" font-weight=\"bold\" fill=\"#212529\">");
    fprintf(f, "SYNTHETIC NETWORK &#8212; educational demo</text>\n");
    if (subtitle != NULL) {
        fprintf(f, "  <text x=\"30\" y=\"60\" font-family=\"Arial, sans-serif\" font-size=\"14\" fill=\"#6c757d\">%s</text>\n", subtitle);
    }

    /* Draw edges */
    for (int e = 0; e < graph->n_edges; e++) {
        int u = graph->edges[e].u;
        int v = graph->edges[e].v;
        int x1 = graph->coords_x[u];
        int y1 = graph->coords_y[u];
        int x2 = graph->coords_x[v];
        int y2 = graph->coords_y[v];
        bool active = graph->edges[e].active;

        /* Check if this edge is on the route path */
        int route_hop = -1;
        if (route != NULL && route->found && route->hop_count > 0) {
            for (int h = 0; h < route->hop_count; h++) {
                int ru = route->path[h];
                int rv = route->path[h + 1];
                if ((ru == u && rv == v) || (ru == v && rv == u)) {
                    route_hop = h + 1;
                    break;
                }
            }
        }

        if (!active) {
            /* Disabled edge: dashed red stroke, with explicit 'closed' text label */
            fprintf(f, "  <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#dc3545\" stroke-width=\"2\" stroke-dasharray=\"6,4\" />\n",
                    x1, y1, x2, y2);
            int mx = (x1 + x2) / 2;
            int my = (y1 + y2) / 2;
            fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"50\" height=\"18\" rx=\"3\" fill=\"#dc3545\" />\n", mx - 25, my - 9);
            fprintf(f, "  <text x=\"%d\" y=\"%d\" font-family=\"Arial, sans-serif\" font-size=\"11\" fill=\"#ffffff\" font-weight=\"bold\" text-anchor=\"middle\">closed</text>\n",
                    mx, my + 4);
        } else if (route_hop > 0) {
            /* Route edge: thick blue stroke with numbered hop marker */
            fprintf(f, "  <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#0d6efd\" stroke-width=\"4\" />\n",
                    x1, y1, x2, y2);
            int mx = (x1 + x2) / 2;
            int my = (y1 + y2) / 2;
            fprintf(f, "  <circle cx=\"%d\" cy=\"%d\" r=\"10\" fill=\"#0d6efd\" />\n", mx, my);
            fprintf(f, "  <text x=\"%d\" y=\"%d\" font-family=\"Arial, sans-serif\" font-size=\"11\" fill=\"#ffffff\" font-weight=\"bold\" text-anchor=\"middle\">%d</text>\n",
                    mx, my + 4, route_hop);
        } else {
            /* Standard active edge: solid neutral dark line */
            fprintf(f, "  <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#495057\" stroke-width=\"2\" />\n",
                    x1, y1, x2, y2);
        }
    }

    /* Draw vertices */
    for (int i = 0; i < graph->n_vertices; i++) {
        int x = graph->coords_x[i];
        int y = graph->coords_y[i];
        bool is_unreached = (unreached != NULL && unreached[i]);

        if (is_unreached) {
            /* Unreached vertex: dashed grey outline with text 'unreached' */
            fprintf(f, "  <circle cx=\"%d\" cy=\"%d\" r=\"20\" fill=\"#e9ecef\" stroke=\"#6c757d\" stroke-width=\"2\" stroke-dasharray=\"4,3\" />\n", x, y);
            fprintf(f, "  <text x=\"%d\" y=\"%d\" font-family=\"Arial, sans-serif\" font-size=\"11\" font-weight=\"bold\" fill=\"#6c757d\" text-anchor=\"middle\">%d</text>\n", x, y + 4, i);
            fprintf(f, "  <text x=\"%d\" y=\"%d\" font-family=\"Arial, sans-serif\" font-size=\"10\" fill=\"#dc3545\" text-anchor=\"middle\">unreached</text>\n", x, y + 33);
        } else {
            /* Standard reached vertex: clean solid node */
            fprintf(f, "  <circle cx=\"%d\" cy=\"%d\" r=\"20\" fill=\"#ffffff\" stroke=\"#212529\" stroke-width=\"2\" />\n", x, y);
            fprintf(f, "  <text x=\"%d\" y=\"%d\" font-family=\"Arial, sans-serif\" font-size=\"12\" font-weight=\"bold\" fill=\"#212529\" text-anchor=\"middle\">%d</text>\n", x, y + 4, i);
            if (graph->labels != NULL && graph->labels[i][0] != '\0') {
                fprintf(f, "  <text x=\"%d\" y=\"%d\" font-family=\"Arial, sans-serif\" font-size=\"12\" fill=\"#212529\" text-anchor=\"middle\">%s</text>\n",
                        x, y + 34, graph->labels[i]);
            }
        }
    }

    fprintf(f, "</svg>\n");
    fclose(f);
    return RG_OK;
}
