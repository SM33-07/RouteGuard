#ifndef RG_SVG_H
#define RG_SVG_H

#include "rg_graph.h"
#include "rg_traverse.h"
#include <stdbool.h>

/**
 * @file rg_svg.h
 * @brief Pure C11 SVG topology export generator using fprintf.
 */

/**
 * @brief Exports graph topology to an SVG file.
 *
 * Visual design rules:
 * - Active edges: solid black line.
 * - Disabled edges: dashed red line with explicit text label "closed" (never color alone).
 * - Route edges: thick blue line with numbered hop markers.
 * - Unreached vertices: dashed grey outline with explicit label "unreached".
 * - Mandatory title banner: "SYNTHETIC NETWORK — educational demo".
 */
RgStatus rg_export_svg(const RgGraph *graph, const RgRoute *route, const bool *unreached, const char *filename, const char *subtitle);

#endif /* RG_SVG_H */
