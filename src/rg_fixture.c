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

static const char *s_campus_descriptions[13] = {
    "VIP Gate",
    "Indian National Flag",
    "Seminar Hall — 2nd Floor, Admin Block (1C)",
    "Old Library — Ground Floor, Admin Block (1C)",
    "GHS Gate",
    "Academic Block 1 (1AB)",
    "Food Court — Old Mess",
    "Academic Block 2 (2AB)",
    "Registration Desk — Ground Floor, opposite Smt. V. Pai Auditorium, 3AB",
    "Smt. Vasanti Pai Auditorium — Ground Floor, 3AB",
    "Conference Hall — 2nd Floor, 3AB",
    "Food Court — LHC",
    "Knowledge Centre"
};

const char *rg_fixture_campus_venue_description(int vertex_id) {
    if (vertex_id < 0 || vertex_id >= 13) {
        return "Unknown Venue";
    }
    return s_campus_descriptions[vertex_id];
}

RgStatus rg_fixture_campus_muj(RgGraph **out_graph) {
    if (out_graph == NULL) {
        return RG_ERR_INVALID_PARAM;
    }

    RgGraph *g = NULL;
    RgStatus s = rg_graph_create(13, 15, &g);
    if (s != RG_OK) {
        return s;
    }

    /* Set 13 vertices: concise display label + spatial coordinates */
    rg_set_vertex(g, 0, "VIP Gate", 120, 620);
    rg_set_vertex(g, 1, "Indian National Flag", 240, 560);
    rg_set_vertex(g, 2, "Seminar Hall (1C)", 360, 680);
    rg_set_vertex(g, 3, "Old Library (1C)", 360, 560);
    rg_set_vertex(g, 4, "GHS Gate", 240, 300);
    rg_set_vertex(g, 5, "Academic Block 1 (1AB)", 480, 460);
    rg_set_vertex(g, 6, "Food Court - Old Mess", 480, 300);
    rg_set_vertex(g, 7, "Academic Block 2 (2AB)", 650, 420);
    rg_set_vertex(g, 8, "Registration Desk (3AB)", 820, 420);
    rg_set_vertex(g, 9, "Smt. V. Pai Auditorium", 940, 520);
    rg_set_vertex(g, 10, "Conference Hall (3AB)", 940, 340);
    rg_set_vertex(g, 11, "Food Court - LHC", 980, 220);
    rg_set_vertex(g, 12, "Knowledge Centre", 1100, 140);

    /* Canonical edge insertion: IDs e0 through e14 */
    rg_add_edge(g, 0, 1, NULL);   /* e0: VIP Gate <-> Indian National Flag */
    rg_add_edge(g, 1, 3, NULL);   /* e1: Indian National Flag <-> Old Library (1C) */
    rg_add_edge(g, 2, 3, NULL);   /* e2: Seminar Hall (1C) <-> Old Library (1C) */
    rg_add_edge(g, 3, 5, NULL);   /* e3: Old Library (1C) <-> 1AB */
    rg_add_edge(g, 4, 5, NULL);   /* e4: GHS Gate <-> 1AB */
    rg_add_edge(g, 4, 6, NULL);   /* e5: GHS Gate <-> Old Mess */
    rg_add_edge(g, 5, 6, NULL);   /* e6: 1AB <-> Old Mess */
    rg_add_edge(g, 5, 7, NULL);   /* e7: 1AB <-> 2AB */
    rg_add_edge(g, 6, 7, NULL);   /* e8: Old Mess <-> 2AB */
    rg_add_edge(g, 7, 8, NULL);   /* e9: 2AB <-> Registration Desk (3AB) - Critical corridor */
    rg_add_edge(g, 8, 9, NULL);   /* e10: Registration Desk <-> Auditorium */
    rg_add_edge(g, 8, 10, NULL);  /* e11: Registration Desk <-> Conference Hall */
    rg_add_edge(g, 9, 10, NULL);  /* e12: Auditorium <-> Conference Hall */
    rg_add_edge(g, 8, 11, NULL);  /* e13: Registration Desk <-> LHC */
    rg_add_edge(g, 11, 12, NULL); /* e14: LHC <-> Knowledge Centre */

    *out_graph = g;
    return RG_OK;
}

