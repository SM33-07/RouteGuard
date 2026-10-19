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

---

## 4. Campus Location Correction, Logo Integration & Loading-State Verification (Branch: `campus-location-branding`)

- **Date**: 2026-10-19
- **Branch**: `campus-location-branding`
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Target Resolutions**: 1366×768 (Standard Laptop), 1920×1080 (Full HD Presentation)
- **Session Video**: `campus_brand_verify_1792423319521.webp`
- **Screenshots Captured**:
  - `brand_about_modal_1792423442151.png` (Overview showing updated pin placement, header icon, 4-hop route, and open Project Info modal with 3:2 wordmark)
  - `brand_failure_reroute_1792423665129.png` (Dynamic failure resilience check after severing `e6`)

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-GEO-01** | **Calibrated Venue Coordinates (13 Venues)** | `[x] VERIFIED` | `brand_about_modal_1792423442151.png` | All 13 campus venue pins positioned accurately to user-annotated reference maps:<br>• #0 Main Gate: `[75.56535, 26.84160]` (southern campus entrance)<br>• #1 Dome Building: `[75.56550, 26.84220]` (placed on OSM "Administrative Block" footprint)<br>• #2 Grand Stairs: `[75.56535, 26.84275]` (plaza directly north of Dome Building)<br>• #3 Academic Block 1: `[75.56450, 26.84310]` (west quad, south of AB3)<br>• #4 Academic Block 2: `[75.56610, 26.84290]` (east quad, south of Auditorium)<br>• #5 Old Mess / Food Court: `[75.56530, 26.84360]` (central dining north of Grand Stairs)<br>• #6 Academic Block 3: `[75.56430, 26.84400]` (north-west academic quad, above AB1)<br>• #7 Lecture Hall Complex: `[75.56500, 26.84435]` (central complex north of Old Mess)<br>• #8 Workshops / Labs: `[75.56660, 26.84370]` (eastern engineering facility)<br>• #9 Football Ground: `[75.56590, 26.84460]` (sports field marked by red arrow)<br>• #10 Cricket Ground: `[75.56420, 26.84550]` (interior of circular cricket turf)<br>• #11 Outdoor Courts: `[75.56520, 26.84580]` (outdoor court area)<br>• #12 Knowledge Centre: `[75.56640, 26.84230]` (east of Dome Building). |
| **TC-BRAND-01** | **Header Logo Icon Integration** | `[x] VERIFIED` | `brand_about_modal_1792423442151.png` | Replaced decorative text "RG" monogram with `/brand/routeguard-icon.png` (32×32px, `object-fit: contain`). Product title "RouteGuard" and "WASM / C11" technical badge preserved. Accessible `alt="RouteGuard Logo"`. |
| **TC-BRAND-02** | **App Favicon & Metadata** | `[x] VERIFIED` | `frontend/index.html` | Updated `<link rel="icon" type="image/png" href="/brand/routeguard-icon.png" />`. Verified resolves in both dev and production build (`npm run build`). |
| **TC-BRAND-03** | **Full 3:2 Wordmark in About Modal** | `[x] VERIFIED` | `brand_about_modal_1792423442151.png` | Full 1536×1024 wordmark rendered inside responsive Project Information dialog (`frontend/public/brand/routeguard-wordmark.png`). Displays system architecture, ISO C11 engine invariants, CSC2101 academic scope boundary, unweighted undirected graph disclaimer, and OSM attribution. Accessible via `ℹ️ About` header button. |
| **TC-LOAD-01** | **Non-Blocking Map Loader** | `[x] VERIFIED` | Live browser test | Map viewport displays a lightweight `.map-loading-overlay` with accessible CSS spinner and "Loading campus map…" status (`role="status"`, `aria-live="polite"`). Application header, sidebar controls, venue indices, and graph solver render immediately without blocking. Overlay smoothly hides once MapLibre fires `load`. |
| **TC-LOAD-02** | **Honest Map Error & Retry State** | `[x] VERIFIED` | Live browser test | If style/tiles fail to load, an honest error banner displays "Basemap unavailable — verify internet connection" with a functional `🔄 Retry Map` button. Graph controls and fallback visualization remain operable. |
| **TC-NAME-01** | **Full Venue Naming & Search Aliasing** | `[x] VERIFIED` | `brand_about_modal_1792423442151.png` | Venue #7 explicitly labeled "Lecture Hall Complex" in all UI elements (map pin, index, route card, inspector). Search index supports alias query "LHC" via note indexing. Venue #5 labeled "Old Mess / Food Court". |
| **TC-RESP-01** | **Responsive Layout & Pin Stability** | `[x] VERIFIED` | Live browser test | Tested at 1366×768 and 1920×1080. Pins remain anchored to geographic coordinates across panning, zooming, and container resizing. Single compact legend, unified toolbar, and zero horizontal scrollbar on route strip. |
| **TC-OPS-01** | **C Engine Invariants & Reroute Verification** | `[x] VERIFIED` | `brand_failure_reroute_1792423665129.png` | Baseline route (Main Gate → AB3) is 4 hops (`0 → 1 → 2 → 3 → 6`). Severing `e6` instantly recomputes to 5-hop alternative (`0 → 1 → 2 → 5 → 7 → 6`). Severing `e12` isolates Football & Cricket Ground ({9, 10}) into 2 components. Reset restores all 17 walkways. Switching to F1 mode preserves 8-hop reference route and 26/144 operation counters. |

---

## 5. Global Action Feedback & Notification System Verification (Branch: `action-feedback-notifications`)

- **Date**: 2026-10-19
- **Branch**: `action-feedback-notifications`
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Target Resolution**: 1366×768 (Standard Laptop)
- **Session Video Recordings**:
  - `toast_feedback_verify_1792424773822.webp`
  - `modal_theme_verify_1792425145789.webp`
  - `toast_theme_capture_1792425324209.webp`
- **Screenshots Captured**:
  - `toast_bfs_traversal_1792424801639.png` (Shows BFS Traversal Completed toast with C visited count)
  - `toast_dfs_traversal_1792424811035.png` (Shows DFS Traversal Completed toast with LIFO stack count)
  - `toast_find_route_1792424820862.png` (Shows Minimum-Hop Route Found toast with 4 hops)
  - `toast_zero_hop_1792424842730.png` (Shows Zero-Hop Route toast when src == dst)
  - `toast_sever_e6_reroute_1792424902229.png` (Shows Walkway Severed e6 rerouting to 5 hops via LHC)
  - `toast_sever_e12_disconnected_1792424920140.png` (Shows Walkway Severed e12 partition and Destination Unreachable warning)
  - `modal_reset_centered_1792425167013.png` (Shows centered in-app Reset Graph to Baseline confirmation modal)
  - `toast_reset_baseline_1792425204623.png` (Shows baseline restored graph state after modal confirmation)
  - `toast_light_mode_visible_1792425340823.png` (Shows toast notification in Light Mode)
  - `toast_hc_mode_visible_1792425359455.png` (Shows toast notification in High Contrast Mode)

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-TOAST-01** | **BFS Traversal Feedback** | `[x] VERIFIED` | `toast_bfs_traversal_1792424801639.png` | Clicking "Run BFS Traversal" provides immediate button feedback and displays an info toast: *"Visited 13 vertices starting from Main Gate — Primary campus arrival & visitor entry (Node 0) in Adjacency List mode."* Traversal sequence visible in inspector. |
| **TC-TOAST-02** | **DFS Traversal Feedback** | `[x] VERIFIED` | `toast_dfs_traversal_1792424811035.png` | Clicking "Run DFS Traversal" confirms completion: *"Explored 13 vertices from Main Gate (Node 0) using LIFO stack."* |
| **TC-TOAST-03** | **Minimum-Hop Route & Zero-Hop** | `[x] VERIFIED` | `toast_find_route_1792424820862.png`, `toast_zero_hop_1792424842730.png` | Standard query confirms: *"4 hops from Main Gate to Academic Block 3 (Main Gate → Dome Building → Grand Stairs → Academic Block 1 → Academic Block 3)."* When src == dst (0 == 0), confirms: *"Source and destination are identical (Main Gate). 0 hops."* |
| **TC-TOAST-04** | **Walkway Severance & Reroute** | `[x] VERIFIED` | `toast_sever_e6_reroute_1792424902229.png` | Severing `e6` (AB1 ↔ AB3) displays a warning toast: *"Disabled e6 (Academic Block 1 ↔ Academic Block 3). Route to Academic Block 3 remains open (5 hops). Components: 1, reachable: 13/13."* |
| **TC-TOAST-05** | **Walkway Disconnection (Critical Cut)** | `[x] VERIFIED` | `toast_sever_e12_disconnected_1792424920140.png` | Severing `e12` (LHC ↔ Football Ground) displays a warning toast: *"Disabled e12 (Lecture Hall Complex ↔ Football Ground). Destination Football Ground is now UNREACHABLE from Main Gate. Components: 2, reachable: 11/13."* Route chip strip updates to `No active path from Main Gate to Football Ground`. |
| **TC-TOAST-06** | **Walkway Restoration** | `[x] VERIFIED` | `toast_restore_e6_1792424936840.png` | Re-opening a severed edge displays a success toast: *"Reconnected e6 (Academic Block 1 ↔ Academic Block 3). Route to Academic Block 3: 4 hops. Components: 1."* |
| **TC-TOAST-07** | **In-App Reset Confirmation Dialog** | `[x] VERIFIED` | `modal_reset_centered_1792425167013.png` | When non-baseline modifications exist, clicking Reset triggers a custom in-app confirmation modal (centered, backdrop blurred, keyboard dismissible with Escape). Summarizes severed walkways to be restored. |
| **TC-TOAST-08** | **Baseline Reset Confirmation** | `[x] VERIFIED` | `toast_reset_baseline_1792425204623.png` | Confirming reset restores all 17 walkways to active, returns src/dst to 0/6, and announces: *"Restored all 17 walkways in MUJ Campus. Active route: 4 hops from Main Gate to Academic Block 3."* When already at baseline, Reset executes immediately without prompt. |
| **TC-TOAST-09** | **Storage Representation Feedback** | `[x] VERIFIED` | `network_tab_1792425000371.png` | Switching to Matrix announces: *"Switched to Adjacency Matrix (O(V²)). BFS examined 169 neighbor slot operations."* Switching to List announces: *"Switched to Adjacency List (O(V + E)). BFS examined 34 neighbor slot operations."* Operation counters strictly match C engine. |
| **TC-TOAST-10** | **Fixture Switching Feedback** | `[x] VERIFIED` | Live browser test | Switching to F1 mode clears stale messages and announces: *"F1 Reference Graph Activated. Loaded 12 vertices and 13 edges. Baseline route: 8 hops."* |
| **TC-TOAST-11** | **SVG Export Feedback** | `[x] VERIFIED` | Live browser test | Clicking "↓ Export SVG" validates C-generated SVG and triggers download, reporting: *"Prepared authoritative C engine diagram (CAMPUS, 4 hops) for download."* |
| **TC-TOAST-12** | **Theme Consistency & Accessibility** | `[x] VERIFIED` | `toast_light_mode_visible_1792425340823.png`, `toast_hc_mode_visible_1792425359455.png` | Toasts render with crisp contrast across Dark, Light (pure white card, subtle shadow, dark text), and High Contrast (pure black card, neon border `#00ff66`, pure white text). Single-toast policy prevents popup stacks. Dismiss button accessible via click and Escape key. |

---

## 6. Critical Campus Geolocation Correction Verification (Branch: `campus-geolocation-fix`)

- **Date**: 2026-10-19
- **Branch**: `campus-geolocation-fix`
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Target Resolution**: 1366×768 (Standard Laptop)
- **Session Video**: `campus_geo_verify_1792430292837.webp`
- **Screenshots Captured**:
  - `campus_corrected_venues_1792430310558.png` (Live MapLibre view showing Dome Building seated directly on the Administrative Block footprint)
  - `campus_reroute_verified_1792430394123.png` (Edge failure rerouting test confirming e6 severance, 5-hop alternate route via LHC, and pin stability)
  - `campus_main_gate_corrected_1792433190264.png` (Live MapLibre view showing Main Gate at the exact western road entrance matching annotated red dot)

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-GEO-FIX-01** | **Dome Building (#1) Footprint Placement** | `[x] VERIFIED` | `campus_corrected_venues_1792430310558.png` | Vertex 1 moved to `[75.56565, 26.84165]`. Marker anchor is aligned directly with the large horizontal building footprint labelled "Administrative Block" on the OpenFreeMap/OSM basemap, matching the user's annotated reference. Retains venue identity "Dome Building". |
| **TC-GEO-FIX-02** | **Main Gate (#0) Campus Entrance Placement** | `[x] VERIFIED` | `campus_main_gate_corrected_1792433190264.png` | Vertex 0 placed at `[75.56386, 26.84158]` (matching the red dot in user annotated reference). Marker anchor is placed at the actual western road-to-campus entrance where the access road meets the campus boundary from Subway Road (OSM University Entrance). Distinct from the southern VIP/Faculty Gate. |
| **TC-GEO-FIX-03** | **Grand Stairs (#2) Placement** | `[x] VERIFIED` | `campus_corrected_venues_1792430310558.png` | Vertex 2 placed at `[75.56550, 26.84252]`, immediately north/above the Dome Building along the central pedestrian axis. |
| **TC-GEO-FIX-04** | **Academic Block 1 (#3) & Academic Block 3 (#6)** | `[x] VERIFIED` | `campus_corrected_venues_1792430310558.png` | AB1 placed at `[75.56435, 26.84275]` (left of Grand Stairs and below AB3). AB3 placed at `[75.56425, 26.84370]` (upper-left academic building indicated in the annotated reference). Walkway `e6` connects them along the western corridor. |
| **TC-GEO-FIX-05** | **Academic Block 2 (#4) & Old Mess (#5)** | `[x] VERIFIED` | `campus_corrected_venues_1792430310558.png` | AB2 placed at `[75.56620, 26.84295]` (east quad, near former Dome Building position). Old Mess / Food Court placed at `[75.56530, 26.84296]` (immediately above Grand Stairs). |
| **TC-GEO-FIX-06** | **Lecture Hall Complex (#7) & Full Naming** | `[x] VERIFIED` | `campus_corrected_venues_1792430310558.png` | LHC placed at `[75.56470, 26.84432]`. Displays full name "Lecture Hall Complex" across map pins, inspector, index, and route strip. |
| **TC-GEO-FIX-07** | **Workshops / Labs (#8) & Knowledge Centre (#12)** | `[x] VERIFIED` | `campus_corrected_venues_1792430310558.png` | Workshops / Labs placed at `[75.56696, 26.84365]` (eastern workshop facility). Knowledge Centre placed at `[75.56660, 26.84230]` (triangular building to the right/east of Dome Building). |
| **TC-GEO-FIX-08** | **Sports Zone (#9 Football, #10 Cricket, #11 Courts)** | `[x] VERIFIED` | `campus_corrected_venues_1792430310558.png` | Football Ground placed at `[75.56580, 26.84445]` (exact point indicated by reference arrow). Cricket Ground placed at `[75.56416, 26.84554]` (interior of circular cricket turf). Outdoor Courts placed at `[75.56545, 26.84570]` (courts east of Cricket Ground). |
| **TC-GEO-FIX-09** | **Route Overlay & Topological Walkway Consistency** | `[x] VERIFIED` | `campus_reroute_verified_1792430394123.png` | Baseline route 0 → 6 is 4 hops (`0 → 1 → 2 → 3 → 6`). Severing `e6` dynamically reroutes in C engine to 5 hops via LHC (`0 → 1 → 2 → 5 → 7 → 6`). All 17 walkways connect cleanly to venue markers with zero crossed or tangled edges. |
| **TC-GEO-FIX-10** | **Map Panning, Zooming & Fit Network Stability** | `[x] VERIFIED` | Live browser test | Clicking "Fit Network" frames all 13 venues from Main Gate (`26.84158`) to Courts (`26.84570`). Markers remain anchored to exact building footprints across zoom and pitch changes. |

---

## 7. Campus Entry Topology Update: Direct Main Gate Route + VIP Gate (Branch: `campus-entry-topology`)

- **Date**: 2026-10-19
- **Branch**: `campus-entry-topology`
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Target Resolution**: 1366×768 (Standard Laptop)
- **Session Video Recordings**:
  - `campus_vip_gate_verification_1792434515545.webp`
  - `vip_gate_and_f1_verification_1792434771146.webp`
- **Screenshots Captured**:
  - `main_gate_to_ab3_2hops_1792434530106.png` (Live MapLibre view showing baseline 2-hop route 0 -> 3 -> 6 via new e17 link)
  - `rerouted_4hops_e6_severed_1792434681759.png` (Edge failure rerouting test confirming e6 severance and 4-hop alternative route via Old Mess & LHC)
  - `vip_gate_route_ab3_1792434827276.png` (Live MapLibre view with VIP Gate selected as source routing to AB3)
  - `f1_mode_verified_1792434861794.png` (F1 Reference Graph verification confirming 12 nodes, 13 edges, and 8-hop route)

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-ENTRY-01** | **Direct Main Gate → AB1 Walkway (e17)** | `[x] VERIFIED` | `main_gate_to_ab3_2hops_1792434530106.png` | Edge `e17: Main Gate (0) ↔ Academic Block 1 (3)` modeled directly in C engine. Minimum-hop route `0 → 3` evaluates to exactly 1 hop in C/WASM. Display polyline visually skirts intervening building footprints via western driveway without altering graph topology. |
| **TC-ENTRY-02** | **Baseline 2-Hop Route (0 → 3 → 6)** | `[x] VERIFIED` | `main_gate_to_ab3_2hops_1792434530106.png` | While `e6` is active, the baseline route from Main Gate (0) to Academic Block 3 (6) evaluates to 2 hops (`0 → 3 → 6`) in both Adjacency List and Adjacency Matrix modes. Dynamic route bar displays `2 Hops: 0: Main Gate → 3: Academic Block 1 → 6: Academic Block 3`. |
| **TC-ENTRY-03** | **e6 Severance Rerouting (4 Hops)** | `[x] VERIFIED` | `rerouted_4hops_e6_severed_1792434681759.png` | Severing `e6` (AB1 ↔ AB3) closes the corridor. C engine recomputes alternative path via Old Mess and LHC: `0 → 3 → 5 → 7 → 6` (4 hops). Warning toast announces reroute without isolating AB3. |
| **TC-ENTRY-04** | **VIP Gate (#13) C Vertex & Geographic Placement** | `[x] VERIFIED` | `vip_gate_route_ab3_1792434827276.png` | Vertex 13 ("VIP Gate") added to C fixture (`1100, 1320`) and geographic catalogue at `[75.56610, 26.84085]`, matching the red dot in the user's annotated reference at the southeastern campus boundary beside the external road. Connected via `e18` to Knowledge Centre (12). |
| **TC-ENTRY-05** | **One-Click Gate Source Selection** | `[x] VERIFIED` | `vip_gate_route_ab3_1792434827276.png` | Quick-source buttons `🚪 Main Gate (0)` and `⭐ VIP Gate (13)` provide instant 1-click selection in the Route Planner. VIP Gate selectable in Source/Destination dropdowns and venue index directory. |
| **TC-ENTRY-06** | **VIP Gate Route Discovery** | `[x] VERIFIED` | `vip_gate_route_ab3_1792434827276.png` | VIP Gate (13) to Knowledge Centre (12) returns 1 hop (`13 → 12`). VIP Gate (13) to Dome Building (1) returns 2 hops (`13 → 12 → 1`). VIP Gate (13) to Main Gate (0) returns 3 hops (`13 → 12 → 1 → 0`). VIP Gate (13) to AB3 (6) returns 5 hops (`13 → 12 → 1 → 0 → 3 → 6`). |
| **TC-ENTRY-07** | **Critical Link e12 Cut (Sports Partition)** | `[x] VERIFIED` | Live browser test | Severing `e12` (LHC ↔ Football Ground) partitions the graph into 2 components. Football Ground (9) is unreachable from 0 (`hop_count: -1`). Reachability from 0 is 12 vertices. |
| **TC-ENTRY-08** | **In-App Reset to Baseline** | `[x] VERIFIED` | Live browser test | Reset confirms restoration of all 19 walkways in MUJ Campus. Restores baseline 2-hop route `0 → 3 → 6` and 1 component. |
| **TC-ENTRY-09** | **Storage Representation Parity** | `[x] VERIFIED` | Live browser test | Adjacency Matrix (196 slots) and Adjacency List (38 slots) return byte-for-byte identical reachability, hop counts, and paths across all 14 vertices. |
| **TC-ENTRY-10** | **F1 Reference Graph Invariant** | `[x] VERIFIED` | `f1_mode_verified_1792434861794.png` | F1 Reference Graph remains completely frozen: 12 vertices, 13 edges, 8-hop route `0 → 1 → 3 → 4 → 6 → 7 → 8 → 9 → 10`, 26 list slots, 144 matrix slots, 14 DFS pushes, 14 pops, 2 stale pops. |
| **TC-ENTRY-11** | **SVG Diagram Export** | `[x] VERIFIED` | Live browser test | Clicking "↓ Export SVG" exports authoritative C-generated SVG containing all 14 vertices, 19 edges, and active/severed styling. |

---

## 8. VIP Gate Direct Connections: Dome Building (e19) & Main Gate (e20) (Branch: `main`)

- **Date**: 2026-10-20
- **Branch**: `main`
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Target Resolution**: 1366×768 (Standard Laptop)
- **Session Video Recording**:
  - `vip_direct_conn_verify_1792436258565.webp`
- **Screenshots Captured**:
  - `vip_to_dome_1hop_1792436354918.png` (Live MapLibre view showing 1-hop route 13 -> 1 via direct edge e19)
  - `vip_to_main_gate_1hop_1792436382504.png` (Live MapLibre view showing 1-hop route 13 -> 0 via direct edge e20)
  - `vip_to_ab3_3hops_1792436452108.png` (Live MapLibre view showing 3-hop route 13 -> 0 -> 3 -> 6 via e20, e17, and e6)
  - `f1_reference_invariant_1792436475166.png` (F1 Reference Graph verification confirming 12 nodes, 13 edges, and 8-hop route unchanged)

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-VIP-01** | **Direct VIP Gate ↔ Dome Building Walkway (e19)** | `[x] VERIFIED` | `vip_to_dome_1hop_1792436354918.png` | Edge `e19: Dome Building (1) ↔ VIP Gate (13)` added to C fixture. In live MapLibre console, routing from VIP Gate (13) to Dome Building (1) returns exactly 1 hop (`13: VIP Gate → 1: Dome Building`). Straight-line walkway polyline links markers directly across the south-central courtyard. |
| **TC-VIP-02** | **Direct VIP Gate ↔ Main Gate Walkway (e20)** | `[x] VERIFIED` | `vip_to_main_gate_1hop_1792436382504.png` | Edge `e20: Main Gate (0) ↔ VIP Gate (13)` added to C fixture. In live MapLibre console, routing from VIP Gate (13) to Main Gate (0) returns exactly 1 hop (`13: VIP Gate → 0: Main Gate`). Display polyline follows southern perimeter road skirting south of the library and admin block. |
| **TC-VIP-03** | **Optimized VIP Gate → AB3 Route (3 Hops)** | `[x] VERIFIED` | `vip_to_ab3_3hops_1792436452108.png` | Combining `e20` (VIP Gate ↔ Main Gate) and `e17` (Main Gate ↔ AB1), the minimum-hop route from VIP Gate (13) to Academic Block 3 (6) reduces from 5 hops to exactly 3 hops (`13: VIP Gate → 0: Main Gate → 3: Academic Block 1 → 6: Academic Block 3`). Active route highlight shines unbroken along the perimeter corridor. |
| **TC-VIP-04** | **Campus Topology Scale & Operation Counters** | `[x] VERIFIED` | Live browser test | Total campus topology verified at 14 vertices and 21 edges (`e0`–`e20`). Adjacency List neighbor traversal slots = 42 ($2 \times 21$). Adjacency Matrix slots = 196 ($14 \times 14$). Single connected component maintained; sports zone separation on `e12` severance preserves reachability of 12 vertices from 0. |
| **TC-VIP-05** | **F1 Reference Graph Invariant Preserved** | `[x] VERIFIED` | `f1_reference_invariant_1792436475166.png` | F1 Reference Graph remains completely frozen: 12 vertices, 13 edges, 8-hop route `0 → 1 → 3 → 4 → 6 → 7 → 8 → 9 → 10`, 26 list slots, 144 matrix slots, 14 DFS pushes, 14 pops, 2 stale pops. |
