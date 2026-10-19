#ifndef RG_FIXTURE_H
#define RG_FIXTURE_H

#include "rg_graph.h"
#include <stddef.h>

/**
 * @file rg_fixture.h
 * @brief Benchmark and edge-case graph fixtures (F1 through F5).
 */

/**
 * @brief Fixture F1 "Campus": 12 vertices, 13 edges, baseline component = 1.
 */
RgStatus rg_fixture_f1_campus(RgGraph **out_graph);

/**
 * @brief Fixture F2: 7 vertices, 4 edges, 4 components (vertices 5 & 6 isolated).
 */
RgStatus rg_fixture_f2_disconnected(RgGraph **out_graph);

/**
 * @brief Fixture F3: 1 vertex, 0 edges (single vertex base case).
 */
RgStatus rg_fixture_f3_single_vertex(RgGraph **out_graph);

/**
 * @brief Fixture F4: 4 vertices, 3 edges in linear chain (0-1-2-3).
 */
RgStatus rg_fixture_f4_chain(RgGraph **out_graph);

/**
 * @brief Fixture F5: 4 vertices, 4 edges in a cycle (0-1-2-3-0) for tie-break testing.
 */
RgStatus rg_fixture_f5_cycle4(RgGraph **out_graph);

/**
 * @brief Fixture MUJ Campus Demo: 14 vertices, 21 edges, baseline component = 1.
 */
RgStatus rg_fixture_campus_muj(RgGraph **out_graph);

/**
 * @brief Retrieves full descriptive name/floor information for a campus vertex (0..13).
 */
const char *rg_fixture_campus_venue_description(int vertex_id);

#endif /* RG_FIXTURE_H */

