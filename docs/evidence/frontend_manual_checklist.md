# RouteGuard Frontend Manual Browser Verification Checklist

This checklist documents the end-to-end verification of the RouteGuard presentation layer in a live browser (`http://localhost:5173/`).
Every verified item was directly tested, observed, and recorded during live browser subagent sessions on the `frontend-wasm` branch.

- **Date**: 2026-10-17
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Engine**: ISO C11 WebAssembly (`routeguard.js`, embedded base64 WASM)
- **WASM Module SHA-256**: `2AACE01A82DFE5C99C8C8256D57357F3A2C5B435A9411D06D5FFC786D6018068`
- **Video Session Recordings**:
  - MapLibre Geospatial Console & 13-Venue Verification: `campus_simulator_verification_1792244696804.webp`
  - Previous Session UI & Algorithms: `muj_campus_frontend_verification_1792241951515.webp`
  - Themes & Architecture Gallery: `themes_and_gallery_verification_1792242726503.webp`

---

## 1. Verification Matrix

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-UI-01** | **Gate A: MapLibre Vector Basemap** | `[x] VERIFIED` | `initial_page_load_1792244709408.png` | MapLibre GL vector basemap cleanly rendered using OpenFreeMap liberty style (`https://tiles.openfreemap.org/styles/liberty`), centered on MUJ (`[75.565234, 26.843855]`), pitch 25°, bearing -15°. Attribution: `OpenFreeMap © OpenStreetMap contributors`. |
| **TC-UI-02** | **13 C-Owned Campus Venues** | `[x] VERIFIED` | `initial_page_load_1792244709408.png` | All 13 venues (0–12) displayed as interactive custom DOM map marker pins with category icons, venue ID, labels, and status badges. |
| **TC-UI-03** | **Knowledge Centre Venue #12** | `[x] VERIFIED` | Live browser test | Node #12 ("Knowledge Centre", digital library) is placed northeast of Dome Building and south of Academic Block 2. Connected via `e15: (1, 12)` and `e16: (4, 12)`. Search filter finds it, clicking centers the camera. |
| **TC-UI-04** | **Gate B: Live C/WASM Traversal** | `[x] VERIFIED` | `initial_page_load_1792244709408.png` | Boot calls C `rg_wasm_init_fixture(1)`. BFS order from 0: `0, 1, 2, 12, 3, 4, 5, 6, 8, 7, 11, 9, 10` (all 13 nodes visited). Default minimum-hop route to 6: `0 → 1 → 2 → 3 → 6` (4 hops). |
| **TC-UI-05** | **13-Venue Campus Counters** | `[x] VERIFIED` | Live browser test | Adjacency List: 34 slots examined ($2E = 2 \times 17 = 34$), 13 vertices dequeued, 0 inactive skips.<br>Adjacency Matrix: 169 slots examined ($V^2 = 13 \times 13 = 169$), 13 vertices dequeued. |
| **TC-UI-06** | **Searchable Venue Index** | `[x] VERIFIED` | Live browser test | Right inspector "📑 Index" tab provides real-time search input. Typing "Knowledge" filters list to Knowledge Centre #12. Clicking the item selects the venue, opens Venue tab, and smoothly centers the MapLibre camera. |
| **TC-UI-07** | **Collapsible Sidebars & Reflow** | `[x] VERIFIED` | Live browser test | Clicking `☰` toggles the Left Control Panel (280px ↔ 0px). Clicking `📊` toggles the Right Inspector (320px ↔ 0px). Center MapLibre container reflows smoothly via ResizeObserver without distortion or blank strips. |
| **TC-UI-08** | **Campus Redundant Edge Reroute (`e6: 3-6`)** | `[x] VERIFIED` | Live browser test | Clicking "Sever e6" closes walkway `e6` (AB1 ↔ AB3). Edge turns into dashed red line with CLOSED tag. Route dynamically recomputes in C engine to route via Old Mess and LHC (`0 → 1 → 2 → 5 → 7 → 6`, 5 hops). |
| **TC-UI-09** | **Campus Critical Cut (`e12: 7-9`)** | `[x] VERIFIED` | Live browser test | Severing `e12` (LHC ↔ Football Ground) partitions the sports zone: Route to 9 is blocked (`UNREACHABLE`), components = 2 (Main campus: 11 nodes, Sports: {9, 10} 2 nodes), reachable from 0 = 11 / 13. |
| **TC-UI-10** | **Campus Reset Graph** | `[x] VERIFIED` | Live browser test | Clicking Reset Graph restores all 17 walkways to active, component count to 1, and 4-hop route `0 → 1 → 2 → 3 → 6`. |
| **TC-UI-11** | **F1 Reference Graph: Mode Switch** | `[x] VERIFIED` | Live browser test | Clicking "📐 F1 Reference Graph" calls `rg_wasm_init_fixture(0)` and switches view to the 12-node F1 SVG topology. Route from 0 to 10 is 8 hops (`0 → 1 → 3 → 4 → 6 → 7 → 8 → 9 → 10`). |
| **TC-UI-12** | **F1 Reference Graph: Counters Parity** | `[x] VERIFIED` | Live browser test | In F1 mode, Adjacency List slots examined = 26 ($2E = 26$). Adjacency Matrix slots examined = 144 ($V^2 = 144$). Fully maintains regression invariants. |
| **TC-UI-13** | **Single-Edge Failure Simulation Table** | `[x] VERIFIED` | Live browser test | Table in Network tab displays all single-edge failure simulations. Explicitly documented as repeated C BFS simulations ($O(E \cdot (V+E))$), strictly avoiding Tarjan/low-link bridge algorithms. |
| **TC-UI-14** | **Theme Switcher** | `[x] VERIFIED` | Live browser test | All 3 themes verified: Dark (slate/graphite), Light (clean slate), and High Contrast (pure black/white/yellow). Map markers and text remain high-contrast and legible in all themes. |
| **TC-UI-15** | **No "Getting DOM..." Placeholder** | `[x] VERIFIED` | Live browser audit | Thoroughly audited: no "Getting DOM..." element or text exists anywhere in the application. Identified as a transient artifact of external browser recording tools. |
| **TC-UI-16** | **Console & Runtime Integrity** | `[x] VERIFIED` | Live browser audit | 0 unhandled exceptions, 0 module resolution errors, 0 failed network requests. |

---

## 2. Technical Observations & Verification Honesty

### 1. Operation Counters vs Storage Distinctions
- **Adjacency List Counters**: `neighbor_slots_examined` strictly reflects algorithm operations during BFS traversal (advancing through the linked list of incident edges). In a connected baseline BFS, this equals $\sum \text{deg}(v) = 2E$. For F1 ($E=13$), counter = 26. For MUJ Campus ($E=17$), counter = 34.
- **Adjacency Matrix Counters**: `neighbor_slots_examined` strictly reflects row-column cell lookups in the matrix during BFS. For a complete search visiting all $V$ nodes, this equals $V \times V = V^2$. For F1 ($V=12$), counter = 144. For MUJ Campus ($V=13$), counter = 169.
- Neither metric was mislabeled as a storage size; both are dynamically reported by the C engine across the WASM bridge.

### 2. Failure Simulation Terminology
- Feature title: **Single-Edge Failure Simulation**.
- Clear academic boundary notice: *"Systematically evaluates network resilience by simulating the temporary closure of each edge individually and executing repeated C BFS traversals ($O(E \cdot (V+E))$). Reports the operational consequences of each simulated closure on destination reachability and connected components without using low-link bridge detection algorithms."*
- Fully compliant with CSC2101 Lecture 37 scope (no Tarjan, Kosaraju, or low-link algorithms).

---

## 3. Final Map-First UI/UX Polish Verification (Branch: `ui-polish`)

- **Date**: 2026-10-18
- **Branch**: `ui-polish`
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Target Resolutions**: 1366×768 (Standard Laptop), 1920×1080 (Presentation Display)
- **Session Video**: `routeguard_final_ui_polish_verification_1792271832668.webp`

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-POLISH-01** | **Single Compact Legend** | `[x] VERIFIED` | `polish_1366x768_baseline_1792271847661.png` | Exactly one campus graph legend anchored at bottom-right inside map workspace. Explains active walkway, severed walkway, minimum-hop route, and source/dest pins with distinctive colors and stroke styles. Duplicate legend eliminated. |
| **TC-POLISH-02** | **Route Summary Bar Redesign** | `[x] VERIFIED` | `polish_1366x768_baseline_1792271847661.png` | Bottom route strip redesigned into compact inline card inside map. Hop count indicator, source/dest badges, and ordered node sequence wrap cleanly with 0 horizontal scrollbar at 1366×768 and 1920×1080. |
| **TC-POLISH-03** | **Decluttered Venue Labels** | `[x] VERIFIED` | `polish_1366x768_baseline_1792271847661.png` | Marker label visibility policy: source, destination, and highlighted route vertices show persistent labels. Non-route vertices display clean circular pins with full labels revealed on hover/focus/selection, preventing label collisions around AB1, Old Mess, Dome, Grand Stairs, and Knowledge Centre. |
| **TC-POLISH-04** | **Sidebar Button & Label Fit** | `[x] VERIFIED` | `polish_1366x768_baseline_1792271847661.png` | Scenario buttons (`Sever e6`, `Sever e12`, `Restore All`) fit comfortably in 280px left panel without clipping or text overflow. Full scenario descriptions provided via native `title` tooltips. |
| **TC-POLISH-05** | **Right Inspector Tab Spacing** | `[x] VERIFIED` | `polish_1366x768_baseline_1792271847661.png` | Tabs (`🏢 Venue`, `🧭 Route`, `📊 Network`, `📑 Index`) fit container width without crowding or wrapping. |
| **TC-POLISH-06** | **Unified Map Floating Toolbar** | `[x] VERIFIED` | `polish_1366x768_baseline_1792271847661.png` | Basemap toggle, "Fit Network", and "Reset View" unified into a clean floating pill toolbar at top-left away from venue labels. Old schematic modal button removed from primary toolbar. |
| **TC-POLISH-07** | **Interactive Failure & Reroute** | `[x] VERIFIED` | `polish_1366x768_failure_1792271870653.png` | Severing `e6` dynamically closes walkway in C engine, renders dashed red edge, and computes alternative path via LHC (`0 → 1 → 2 → 5 → 7 → 6`, 5 hops). |
| **TC-POLISH-08** | **Full HD Presentation View** | `[x] VERIFIED` | `polish_1920x1080_presentation_1792271926675.png` | Clean presentation layout at 1920×1080 with comfortable contrast, balanced spacing, and zero layout breakage. |
| **TC-POLISH-09** | **Academic & C11 Engine Parity** | `[x] VERIFIED` | `bin/rg_tests.exe`, `bin/rg_bridge_test.exe` | 28/28 native unit tests passed; isolated bridge contract passed. Zero algorithmic code written in TypeScript; all traversals, routes, reachability, and counters are 100% C-authoritative. |

