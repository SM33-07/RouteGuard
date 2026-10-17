# RouteGuard Frontend Manual Browser Verification Checklist

This checklist documents the end-to-end verification of the RouteGuard presentation layer in a live browser (`http://localhost:5173/`).
Every verified item was directly tested, observed, and recorded during live browser subagent sessions on the `frontend-wasm` branch.

- **Date**: 2026-10-17
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Engine**: ISO C11 WebAssembly (`routeguard.js`, embedded base64 WASM)
- **WASM Module SHA-256**: `2E55A632A6D527033D3260D32EDFA4C5CF33C2A86EC70196FF9412A6DF5EF812`
- **Video Session Recordings**:
  - Primary UI & Algorithms: `muj_campus_frontend_verification_1792241951515.webp`
  - Themes & Architecture Gallery: `themes_and_gallery_verification_1792242726503.webp`

---

## 1. Verification Matrix

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-UI-01** | **Gate 1: Campus Default Baseline** | `[x] VERIFIED` | `initial_load_1792241962762.png` | MUJ Campus Demo loads by default with 12 C-owned venues, 15 edges. Default src=0 (Main Gate), dst=6 (Academic Block 3), route=4 hops (`0 → 1 → 2 → 3 → 6`), 1 component. |
| **TC-UI-02** | **Primary Schematic Overlay** | `[x] VERIFIED` | `initial_load_1792241962762.png` | High-resolution landscape schematic (`/maps/muj-campus-wayfinding-schematic.png`, 2048x1448) cleanly rendered under SVG nodes/edges without distortion. Landmark index 1–8 clearly visible. |
| **TC-UI-03** | **Campus Disclaimers Display** | `[x] VERIFIED` | `initial_load_1792241962762.png` | Banner: *"SYNTHETIC EDUCATIONAL NETWORK: Pedagogical model for CSC2101/CSC2130. Route length = hops. Not for real-world navigation or municipal transit."*<br>Campus disclosure: *"Illustrative graph model — connections are simplified for DSA demonstration and are not verified pedestrian directions."* |
| **TC-UI-04** | **Gate 1: Campus BFS Traversal Order** | `[x] VERIFIED` | `bfs_run_result_1792242004382.png` | C-computed BFS visit sequence from vertex 0: `0: Main Gate`, `1: Dome Building`, `2: Grand Stairs`, `3: Academic Block 1`, `4: Academic Block 2`, `5: Old Mess / Food Court`, `6: Academic Block 3`, `8: Workshops / Labs`, `7: Lecture Hall Complex`, `11: Outdoor Courts`, `9: Football Ground`, `10: Cricket Ground` (12 nodes visited). |
| **TC-UI-05** | **Campus Baseline Counters** | `[x] VERIFIED` | `counters_checked_1792242102643.png` | Adjacency List: 30 slots examined ($2E = 30$), 12 vertices dequeued, 0 inactive skips.<br>Adjacency Matrix: 144 slots examined ($V^2 = 12 \times 12 = 144$), 12 vertices dequeued. |
| **TC-UI-06** | **Campus DFS Traversal & Stale Pops** | `[x] VERIFIED` | `dfs_sequence_view_1792242229227.png` | C iterative DFS traversal with explicit LIFO stack and mark-on-pop exploration sequence displayed in sequence chips. |
| **TC-UI-07** | **Campus Redundant Edge Reroute (`e6: 3-6`)** | `[x] VERIFIED` | Live browser test | Walkway `e6` toggled to closed. Route from 0 to 6 automatically reroutes via Old Mess and LHC: `0 → 1 → 2 → 5 → 7 → 6` (5 hops). All 12 nodes remain connected (1 component). |
| **TC-UI-08** | **Campus Critical Corridor Cut (`e12: 7-9`)** | `[x] VERIFIED` | `sever_e12_effect_1792242329931.png` | Severing `e12` (LHC ↔ Football Ground) partitions the network: Route to 9 is blocked (`UNREACHABLE`), components = 2 (`Comp 0`: 10 vertices, `Comp 1`: {9, 10} 2 vertices), reachable from 0 = 10 / 12. Nodes 9 & 10 display explicit `UNREACHED` badges. |
| **TC-UI-09** | **Campus Reset Graph** | `[x] VERIFIED` | `graph_reset_verified_1792242371833.png` | Clicking Reset Graph restores all 15 walkways to active, component count to 1, and 4-hop route `0 → 1 → 2 → 3 → 6`. |
| **TC-UI-10** | **F1 Reference Graph: Baseline Route & BFS** | `[x] VERIFIED` | `media_1792242566783.png` | Switching to F1 mode loads 12 vertices, 13 edges. BFS order from 0: `0, 1, 2, 11, 3, 4, 5, 6, 7, 8, 9, 10`. Minimum-hop route to 10: `0 → 1 → 3 → 4 → 6 → 7 → 8 → 9 → 10` (8 hops). |
| **TC-UI-11** | **F1 Reference Graph: Counters Parity** | `[x] VERIFIED` | Live browser test | Adjacency List slots examined: 26 ($2E = 26$). Adjacency Matrix slots examined: 144 ($V^2 = 144$). Identical 12 nodes processed and queue enqueues/dequeues. |
| **TC-UI-12** | **F1 Reference Graph: Cut `e9: 6-7`** | `[x] VERIFIED` | Live browser test | Disabling `e9` cuts destination 10: destination unreachable, 2 components, exactly 8 reachable vertices from 0. List slots = 19, matrix slots = 96, 1 inactive skip. |
| **TC-UI-13** | **Single-Edge Failure Simulation Table** | `[x] VERIFIED` | `media_1792242615922.png` | Table lists each single-edge simulated failure. Terminology strictly describes repeated single-edge simulation via C BFS ($O(E \cdot (V+E))$) with columns for edge, connection, status, destination reachability, hops, and components. |
| **TC-UI-14** | **Light Theme Toggle** | `[x] VERIFIED` | `theme_light_mode_1792242763285.png` | Crisp daylight operations theme: slate-white surfaces, dark slate text (`#0f172a`), readable node contrast, clear borders. |
| **TC-UI-15** | **High-Contrast Theme Toggle (WCAG AAA)** | `[x] VERIFIED` | `theme_high_contrast_mode_1792242798297.png` | Pure black backgrounds (`#000000`), pure white text and borders (`#ffffff`), high-visibility yellow active indicators (`#ffff00`). |
| **TC-UI-16** | **Reference Maps & Satellite Gallery Modal** | `[x] VERIFIED` | `gallery_modal_satellite_tab_1792242919312.png` | Modal opens via `#btn-open-gallery`. Verified all 4 tabs: Site Master Plan (Hafeez Contractor), AB1 & Old Mess Satellite, LHC & AIC-MUJ Satellite, Football & Sports Satellite with architectural captions. |
| **TC-UI-17** | **Console & Runtime Integrity** | `[x] VERIFIED` | Live browser audit | 0 unhandled exceptions, 0 module resolution errors, 0 failed network requests. |

---

## 2. Detailed Technical Observations

### 1. Operation Counters vs Storage Distinctions
- **Adjacency List Counters**: `neighbor_slots_examined` strictly reflects algorithm operations during BFS traversal (advancing through the linked list of incident edges). In a connected baseline BFS, this equals $\sum \text{deg}(v) = 2E$. For F1 ($E=13$), counter = 26. For MUJ Campus ($E=15$), counter = 30.
- **Adjacency Matrix Counters**: `neighbor_slots_examined` strictly reflects row-column cell lookups in the matrix during BFS. For a complete search visiting all $V$ nodes, this equals $V \times V = V^2$. For F1 ($V=12$), counter = 144. For MUJ Campus ($V=12$), counter = 144.
- Neither metric was mislabeled as a storage size; both are dynamically reported by the C engine across the WASM bridge.

### 2. Failure Simulation Terminology
- Feature title: **Single-Edge Failure Simulation**.
- Clear academic boundary notice: *"Systematically evaluates network resilience by simulating the temporary closure of each edge individually and executing repeated C BFS traversals ($O(E \cdot (V+E))$). Reports the operational consequences of each simulated closure on destination reachability and connected components without using low-link bridge detection algorithms."*
- Fully compliant with CSC2101 Lecture 37 scope (no Tarjan, Kosaraju, or low-link algorithms).
