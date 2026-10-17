# RouteGuard Frontend Manual Browser Verification Checklist

This checklist documents the end-to-end verification of the RouteGuard presentation layer in a live browser (`http://localhost:5173/`).
Every verified item was directly tested and recorded during automated browser subagent sessions.

- **Date**: 2026-10-17
- **Browser**: Headless Chromium (Agentic browser subagent)
- **Engine**: ISO C11 WebAssembly (`routeguard.js`, embedded base64 WASM, SHA-256: `C230E55772A8B35BAE0687346DEB8AFD64D5455CE1B738270A04204CD65B09D8`)
- **Video Session Recording**: `theme_and_e9_audit_1792195694227.webp`

---

## Verification Matrix

| Check ID | Verification Item | Status | Screenshot Artifact | Observed Evidence |
| :--- | :--- | :---: | :--- | :--- |
| **TC-UI-01** | **Initial Baseline Load** | `[x] VERIFIED` | `audit_initial_dark_1792195400762.png` | 12 vertices, 13 edges, default src=0 (Main Gate), dst=10 (Medical Annex), route=8 hops (`0 → 1 → 3 → 4 → 6 → 7 → 8 → 9 → 10`), 1 component. |
| **TC-UI-02** | **BFS Traversal Order** | `[x] VERIFIED` | `audit_bfs_run_1792195570582.png` | Sequence: `0, 1, 2, 11, 3, 4, 5, 6, 7, 8, 9, 10` (12 nodes visited). Queue enqueues/dequeues = 12/12. |
| **TC-UI-03** | **DFS Traversal Order & Stale Pops** | `[x] VERIFIED` | `audit_dfs_run_1792195585557.png` | Sequence: `0, 1, 3, 4, 6, 7, 8, 9, 10, 5, 2, 11`. Mark-on-pop LIFO stack exploration produces exactly `2` stale pops. |
| **TC-UI-04** | **Baseline Counters: Adjacency List** | `[x] VERIFIED` | `audit_ccounters_opened_1792195646479.png` | 26 neighbor slots examined ($2E = 26$), 12 vertices dequeued, 0 closed edges encountered, 0 stale pops. |
| **TC-UI-05** | **Baseline Counters: Adjacency Matrix** | `[x] VERIFIED` | `audit_adj_matrix_1792195533430.png` | 144 neighbor slots examined ($V^2 = 12 \times 12 = 144$), 12 vertices dequeued, 0 closed edges encountered. |
| **TC-UI-06** | **Critical Link Cut (`e9: 6-7`) & Unreachable** | `[x] VERIFIED` | `audit_e9_list_counters_1792195779203.png` | Edge `e9` displays dashed line + `CLOSED` badge. Destination `10` becomes `UNREACHABLE`. Reached vertices = 8 (`0, 1, 2, 3, 4, 5, 6, 11`). Unreached vertices (`7, 8, 9, 10`) display dashed outlines and explicit `UNREACHED` badges. Components = 2. |
| **TC-UI-07** | **Counter Audit on Cut `e9` (Adjacency List)** | `[x] VERIFIED` | `audit_e9_list_counters_1792195779203.png` | **19 slots examined** ($3+2+2+1+4+2+2+3=19$), **8 nodes processed**, **1 inactive skip** (edge `e9` incident to node 6). |
| **TC-UI-08** | **Counter Audit on Cut `e9` (Adjacency Matrix)** | `[x] VERIFIED` | `audit_e9_matrix_counters_1792195793141.png` | **96 slots examined** ($8 \text{ nodes} \times 12 \text{ cols} = 96$), **8 nodes processed**, **1 inactive skip**. |
| **TC-UI-09** | **Critical Link Recovery (`e9`)** | `[x] VERIFIED` | `audit_restored_baseline.png` | Re-enabling `e9` immediately restores the 8-hop route (`0 → 1 → 3 → 4 → 6 → 7 → 8 → 9 → 10`) and reduces component count to 1. |
| **TC-UI-10** | **Redundant Walkway Severed (`e5: 3-4`)** | `[x] VERIFIED` | `audit_failure_table_1792195660575.png` | Severing `e5` reroutes traffic via Science Block B: `0 → 1 → 3 → 5 → 6 → 7 → 8 → 9 → 10` (8 hops). All 12 nodes remain reachable. |
| **TC-UI-11** | **Single-Edge Failure Simulation Table** | `[x] VERIFIED` | `audit_failure_table_1792195660575.png` | 13 rows rendered with columns: `EDGE`, `CONNECTION`, `STATUS`, `DESTINATION REACHABLE?`, `HOPS`, `CONNECTED COMPONENTS`. Terminology describes repeated single-edge simulation; no bridge algorithm claims. |
| **TC-UI-12** | **Light Theme Toggle** | `[x] VERIFIED` | `audit_light_theme_verified_1792195709642.png` | Crisp daylight operations theme: slate-white surfaces, dark slate text (`#0f172a`), readable node contrast, clear borders. |
| **TC-UI-13** | **High-Contrast Theme Toggle (WCAG AAA)** | `[x] VERIFIED` | `audit_high_contrast_verified_1792195716256.png` | Pure black backgrounds (`#000000`), pure white text and borders (`#ffffff`), yellow active indicators (`#ffff00`). |
| **TC-UI-14** | **Graph Reset & Defaults** | `[x] VERIFIED` | `audit_restored_baseline.png` | Reset restores original F1 state: 13 active edges, 0 closed, default source 0, default destination 10, 1 component. |
| **TC-UI-15** | **SVG File Export** | `[x] VERIFIED` | Direct download / verification | C-generated SVG includes XML declaration, synthetic educational disclaimer, active edges, closed edges, and route highlights. |
| **TC-UI-16** | **Console & Runtime Integrity** | `[x] VERIFIED` | Browser console audit | 0 unhandled exceptions, 0 module resolution errors, 0 failed network requests. |

---

## Detailed Observations

### 1. Counter Derivation Verification with `e9` Disabled
When walkway `e9` (connecting `6` and `7`) is severed:
- **Vertices in Source Component**: $\{0, 1, 2, 3, 4, 5, 6, 11\}$ (8 vertices).
- **Matrix Counter Derivation**: 8 vertices dequeued $\times 12$ columns scanned per row = **96 slots**.
- **List Counter Derivation**: Sum of degrees of reached vertices:
  - $\text{deg}(0) = 3$ (edges e0, e1, e2)
  - $\text{deg}(1) = 2$ (edges e0, e3)
  - $\text{deg}(2) = 2$ (edges e1, e4)
  - $\text{deg}(11) = 1$ (edge e2)
  - $\text{deg}(3) = 4$ (edges e3, e4, e5, e6)
  - $\text{deg}(4) = 2$ (edges e5, e7)
  - $\text{deg}(5) = 2$ (edges e6, e8)
  - $\text{deg}(6) = 3$ (edges e7, e8, e9)
  - Total list slots examined = $3 + 2 + 2 + 1 + 4 + 2 + 2 + 3 =$ **19 slots**.
- **Inactive Skips**: Exactly **1** (edge `e9` incident to vertex 6).
- **Live C Engine Output**: Matched 96 (matrix), 19 (list), 8 (dequeued), and 1 (inactive skip) exactly.

### 2. Failure Simulation Terminology
- Feature title: **Single-Edge Failure Simulation**.
- Description: *"Systematically evaluates network resilience by simulating the temporary closure of each edge individually and executing repeated C BFS traversals ($O(E \cdot (V+E))$). Reports the operational consequences of each simulated closure on destination reachability and connected components without using low-link bridge detection algorithms."*
- Column headers: `Destination Reachable?` and `Connected Components`.
- Consequence states: `Reachable (Redundant)` vs. `Unreachable (Severed)`.

### 3. Theme Accessibility
- **Dark Mode**: Deep graphite/navy background (`#080c14`), cyan accents (`#38bdf8`), subtle slate borders (`#334155`).
- **Light Mode**: Off-white canvas (`#f8fafc`), pure white card surfaces (`#ffffff`), dark slate typography (`#0f172a`), deep sky-blue accents (`#0284c7`).
- **High-Contrast Mode**: Pure black (`#000000`), white 1.5px/2px borders (`#ffffff`), high-visibility yellow highlights (`#ffff00`), and crimson alert badges (`#ff3333`).
