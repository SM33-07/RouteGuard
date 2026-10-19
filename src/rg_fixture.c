#include "rg_fixture.h"

RgStatus rg_fixture_f1_campus(RgGraph **out_graph) {
    if (out_graph == NULL) {
        return RG_ERR_INVALID_PARAM;
    }

    RgGraph *g = NULL;
    RgStatus s = rg_graph_create(12, 13, &g);
    if (s != RG_OK) {
        return s;
    }

    /* Vertex setup with display coordinates for readable SVG layout */
    rg_set_vertex(g, 0, "Main Gate", 50, 200);
    rg_set_vertex(g, 1, "Admin Block", 180, 100);
    rg_set_vertex(g, 2, "Library", 180, 300);
    rg_set_vertex(g, 3, "Central Plaza", 320, 200);
    rg_set_vertex(g, 4, "Block A", 460, 120);
    rg_set_vertex(g, 5, "Block B", 460, 280);
    rg_set_vertex(g, 6, "Hostel Junction", 600, 200);
    rg_set_vertex(g, 7, "Sports Complex", 720, 200);
    rg_set_vertex(g, 8, "Auditorium", 840, 200);
    rg_set_vertex(g, 9, "Health Centre", 960, 200);
    rg_set_vertex(g, 10, "Medical Annex", 1080, 200);
    rg_set_vertex(g, 11, "Parking", 80, 350);

    /* Canonical edge insertion: IDs e0 through e12 */
    rg_add_edge(g, 0, 1, NULL);   /* e0 */
    rg_add_edge(g, 0, 2, NULL);   /* e1 */
    rg_add_edge(g, 0, 11, NULL);  /* e2 */
    rg_add_edge(g, 1, 3, NULL);   /* e3 */
    rg_add_edge(g, 2, 3, NULL);   /* e4 */
    rg_add_edge(g, 3, 4, NULL);   /* e5 */
    rg_add_edge(g, 3, 5, NULL);   /* e6 */
    rg_add_edge(g, 4, 6, NULL);   /* e7 */
    rg_add_edge(g, 5, 6, NULL);   /* e8 */
    rg_add_edge(g, 6, 7, NULL);   /* e9 */
    rg_add_edge(g, 7, 8, NULL);   /* e10 */
    rg_add_edge(g, 8, 9, NULL);   /* e11 */
    rg_add_edge(g, 9, 10, NULL);  /* e12 */

    *out_graph = g;
    return RG_OK;
}

RgStatus rg_fixture_f2_disconnected(RgGraph **out_graph) {
    if (out_graph == NULL) {
        return RG_ERR_INVALID_PARAM;
    }
    RgGraph *g = NULL;
    RgStatus s = rg_graph_create(7, 4, &g);
    if (s != RG_OK) {
        return s;
    }

    rg_set_vertex(g, 0, "Node 0", 100, 100);
    rg_set_vertex(g, 1, "Node 1", 200, 100);
    rg_set_vertex(g, 2, "Node 2", 150, 200);
    rg_set_vertex(g, 3, "Node 3", 300, 100);
    rg_set_vertex(g, 4, "Node 4", 400, 100);
    rg_set_vertex(g, 5, "Isolated 5", 100, 300);
    rg_set_vertex(g, 6, "Isolated 6", 300, 300);

    rg_add_edge(g, 0, 1, NULL);
    rg_add_edge(g, 1, 2, NULL);
    rg_add_edge(g, 2, 0, NULL);
    rg_add_edge(g, 3, 4, NULL);
    /* 5 and 6 remain isolated with degree 0 */

    *out_graph = g;
    return RG_OK;
}

RgStatus rg_fixture_f3_single_vertex(RgGraph **out_graph) {
    if (out_graph == NULL) {
        return RG_ERR_INVALID_PARAM;
    }
    RgGraph *g = NULL;
    RgStatus s = rg_graph_create(1, 0, &g);
    if (s != RG_OK) {
        return s;
    }
    rg_set_vertex(g, 0, "Single Node", 100, 100);
    *out_graph = g;
    return RG_OK;
}

RgStatus rg_fixture_f4_chain(RgGraph **out_graph) {
    if (out_graph == NULL) {
        return RG_ERR_INVALID_PARAM;
    }
    RgGraph *g = NULL;
    RgStatus s = rg_graph_create(4, 3, &g);
    if (s != RG_OK) {
        return s;
    }
    for (int i = 0; i < 4; i++) {
        char buf[16];
        buf[0] = 'C'; buf[1] = (char)('0' + i); buf[2] = '\0';
        rg_set_vertex(g, i, buf, 100 + i * 100, 100);
    }
    rg_add_edge(g, 0, 1, NULL);
    rg_add_edge(g, 1, 2, NULL);
    rg_add_edge(g, 2, 3, NULL);
    *out_graph = g;
    return RG_OK;
}

RgStatus rg_fixture_f5_cycle4(RgGraph **out_graph) {
    if (out_graph == NULL) {
        return RG_ERR_INVALID_PARAM;
    }
    RgGraph *g = NULL;
    RgStatus s = rg_graph_create(4, 4, &g);
    if (s != RG_OK) {
        return s;
    }
    rg_set_vertex(g, 0, "V0", 100, 100);
    rg_set_vertex(g, 1, "V1", 200, 100);
    rg_set_vertex(g, 2, "V2", 200, 200);
    rg_set_vertex(g, 3, "V3", 100, 200);

    rg_add_edge(g, 0, 1, NULL);
    rg_add_edge(g, 1, 2, NULL);
    rg_add_edge(g, 2, 3, NULL);
    rg_add_edge(g, 3, 0, NULL);
    *out_graph = g;
    return RG_OK;
}

static const char *s_campus_descriptions[14] = {
    "Main Gate — Primary campus arrival & visitor entry",
    "Dome Building — Library, administration, services (Landmark 1)",
    "Grand Stairs / Central Plaza — Central arrival and circulation hub",
    "Academic Block 1 — Left side of the central stairs (Landmark 2)",
    "Academic Block 2 — Right side of the central stairs (Landmark 3)",
    "Old Mess / Food Court — Dining and student seating (Landmark 4)",
    "Academic Block 3 — Arts, media and specialist spaces (Landmark 6)",
    "Lecture Hall Complex — Large teaching halls (Landmark 5)",
    "Workshops / Labs — Workshop-side teaching facilities (Landmark 8)",
    "Football Ground — Sports grounds (Landmark 7)",
    "Cricket Ground — Sports grounds (Landmark 7)",
    "Outdoor Courts / Turfs — Sports grounds (Landmark 7)",
    "Knowledge Centre — Digital library & resource centre (approx. placement)",
    "VIP Gate — Southeastern campus arrival & executive entry"
};

const char *rg_fixture_campus_venue_description(int vertex_id) {
    if (vertex_id < 0 || vertex_id >= 14) {
        return "Unknown Venue";
    }
    return s_campus_descriptions[vertex_id];
}

RgStatus rg_fixture_campus_muj(RgGraph **out_graph) {
    if (out_graph == NULL) {
        return RG_ERR_INVALID_PARAM;
    }

    RgGraph *g = NULL;
    RgStatus s = rg_graph_create(14, 21, &g);
    if (s != RG_OK) {
        return s;
    }

    /* Set 14 vertices: concise label + exact coordinates aligned to 2048x1448 schematic */
    rg_set_vertex(g, 0, "Main Gate", 750, 1260);
    rg_set_vertex(g, 1, "Dome Building", 750, 1090);
    rg_set_vertex(g, 2, "Grand Stairs", 750, 980);
    rg_set_vertex(g, 3, "Academic Block 1", 470, 910);
    rg_set_vertex(g, 4, "Academic Block 2", 980, 910);
    rg_set_vertex(g, 5, "Old Mess / Food Court", 740, 740);
    rg_set_vertex(g, 6, "Academic Block 3", 440, 580);
    rg_set_vertex(g, 7, "Lecture Hall Complex", 800, 580);
    rg_set_vertex(g, 8, "Workshops / Labs", 1200, 750);
    rg_set_vertex(g, 9, "Football Ground", 1190, 560);
    rg_set_vertex(g, 10, "Cricket Ground", 1170, 360);
    rg_set_vertex(g, 11, "Outdoor Courts", 1250, 970);
    rg_set_vertex(g, 12, "Knowledge Centre", 1080, 1080);
    rg_set_vertex(g, 13, "VIP Gate", 1100, 1320);

    /* Canonical edge insertion: IDs e0 through e20 (illustrative modeled walkways) */
    rg_add_edge(g, 0, 1, NULL);   /* e0: Main Gate <-> Dome Building */
    rg_add_edge(g, 1, 2, NULL);   /* e1: Dome Building <-> Grand Stairs */
    rg_add_edge(g, 2, 3, NULL);   /* e2: Grand Stairs <-> Academic Block 1 */
    rg_add_edge(g, 2, 4, NULL);   /* e3: Grand Stairs <-> Academic Block 2 */
    rg_add_edge(g, 2, 5, NULL);   /* e4: Grand Stairs <-> Old Mess / Food Court */
    rg_add_edge(g, 3, 5, NULL);   /* e5: Academic Block 1 <-> Old Mess / Food Court */
    rg_add_edge(g, 3, 6, NULL);   /* e6: Academic Block 1 <-> Academic Block 3 (Redundant corridor) */
    rg_add_edge(g, 4, 5, NULL);   /* e7: Academic Block 2 <-> Old Mess / Food Court */
    rg_add_edge(g, 4, 8, NULL);   /* e8: Academic Block 2 <-> Workshops / Labs */
    rg_add_edge(g, 5, 7, NULL);   /* e9: Old Mess / Food Court <-> Lecture Hall Complex */
    rg_add_edge(g, 5, 8, NULL);   /* e10: Old Mess / Food Court <-> Workshops / Labs */
    rg_add_edge(g, 6, 7, NULL);   /* e11: Academic Block 3 <-> Lecture Hall Complex */
    rg_add_edge(g, 7, 9, NULL);   /* e12: Lecture Hall Complex <-> Football Ground (Critical sports bridge) */
    rg_add_edge(g, 8, 11, NULL);  /* e13: Workshops / Labs <-> Outdoor Courts */
    rg_add_edge(g, 9, 10, NULL);  /* e14: Football Ground <-> Cricket Ground */
    rg_add_edge(g, 1, 12, NULL);  /* e15: Dome Building <-> Knowledge Centre */
    rg_add_edge(g, 4, 12, NULL);  /* e16: Academic Block 2 <-> Knowledge Centre */
    rg_add_edge(g, 0, 3, NULL);   /* e17: Main Gate <-> Academic Block 1 (Direct west link) */
    rg_add_edge(g, 13, 12, NULL); /* e18: VIP Gate <-> Knowledge Centre (Southeastern entry) */
    rg_add_edge(g, 1, 13, NULL);  /* e19: Dome Building <-> VIP Gate (Executive central access) */
    rg_add_edge(g, 0, 13, NULL);  /* e20: Main Gate <-> VIP Gate (Southern perimeter corridor) */

    *out_graph = g;
    return RG_OK;
}

