# RouteGuard — Master Test Plan (SRS v0.2 Aligned)

This test plan defines the formal test-ID contract strictly aligned with **SRS v0.2 Section 10**, with supplemental tests assigned under the `RG-X-##` namespace.

---

## 1. Canonical SRS v0.2 Section 10 Test Suite

| Test ID | Test Function | Source File | Target Requirement / Scenario | Fixture | Expected Result |
|:---|:---|:---|:---|:---:|:---|
| `RG-GR-01` | `test_rg_gr_01` | `tests/test_graph.c` | Load canonical fixture; verify V and E counts; matrix/list agreement | F1 | V=12, E=13; every matrix entry exactly matches adjacency list |
| `RG-GR-02` | `test_rg_gr_02` | `tests/test_graph.c` | Reject out-of-bounds endpoint | F1 | `RG_ERR_INVALID_PARAM`; graph state unmodified |
| `RG-ERR-01` | `test_rg_err_01` | `tests/test_graph.c` | Invalid source/destination handling | F1 | Returns error; zero crashes; clean return status |
| `RG-MEM-01` | `test_rg_mem_01` | `tests/test_graph.c` | Repeated graph lifecycle & allocation fault injection | F1 | Active allocations return to 0; fault injection cleanly handled |
| `RG-BFS-01` | `test_rg_bfs_01` | `tests/test_traverse.c` | BFS traversal from known start (0) | F1 | Matches exact visit order: `0, 1, 2, 11, 3, 4, 5, 6, 7, 8, 9, 10` |
| `RG-BFS-02` | `test_rg_bfs_02` | `tests/test_traverse.c` | Shortest-hop route from 0 to 10 | F1 | Route found: `0, 1, 3, 4, 6, 7, 8, 9, 10` (8 hops) |
| `RG-BFS-03` | `test_rg_bfs_03` | `tests/test_traverse.c` | Unreachable destination handling | F2 | Returns `RG_NO_ROUTE`, `rt->found = false`, `hop_count = -1` |
| `RG-BFS-04` | `test_rg_bfs_04` | `tests/test_traverse.c` | Source equals destination base case | F1 | Returns `RG_OK`, path `[3]`, `hop_count = 0` |
| `RG-DFS-01` | `test_rg_dfs_01` | `tests/test_traverse.c` | Iterative DFS traversal from known start (0) | F1 | Matches exact visit order: `0, 1, 3, 2, 4, 6, 5, 7, 8, 9, 10, 11` |
| `RG-DFS-02` | `test_rg_dfs_02` | `tests/test_traverse.c` | DFS reachability in disconnected graph | F2 | 0->1, 0->2, 3->4 true; 0->3, 0->5, 0->6 false |
| `RG-COMP-01`| `test_rg_comp_01` | `tests/test_analysis.c` | Count connected components on baseline | F1 | Exactly 1 component |
| `RG-FAIL-01`| `test_rg_fail_01` | `tests/test_analysis.c` | Disable critical connecting walkway `e9` (6-7) | F1 | Destination becomes unreachable; reachable=8; components=2 |
| `RG-FAIL-02`| `test_rg_fail_02` | `tests/test_analysis.c` | Disable redundant walkway `e5` (3-4) | F1 | Destination remains reachable (8 hops via Block B); components=1 |
| `RG-FAIL-03`| `test_rg_fail_03` | `tests/test_analysis.c` | Re-enable walkway and reset graph | F1 | Baseline results restored (8 hops via Block A; 1 component) |
| `RG-REP-01` | `test_rg_rep_01` | `tests/test_traverse.c` | Matrix vs List BFS equivalence | F1 | Identical visit order, reachability, and hop distances |
| `RG-REP-02` | `test_rg_rep_02` | `tests/test_traverse.c` | Toggle edge and compare representations | F1 | Both matrix and list reflect identical mutated graph and rerouted path |
| `RG-MET-01` | `test_rg_met_01` | `tests/test_fixture_ref.c` | Fixed fixture run twice | F1 | Operation counters are byte-for-byte identical across runs |

---

## 2. Supplemental Test Suite (`RG-X-##`)

| Test ID | Test Function | Source File | Purpose / Scenario | Fixture | Expected Result |
|:---|:---|:---|:---|:---:|:---|
| `RG-X-01` | `test_rg_x_01` | `tests/test_analysis.c` | Toggle idempotency & non-existent edge handling | F1 | Disabling disabled edge returns `RG_NOOP`; invalid edge returns `RG_ERR_NOT_FOUND` |
| `RG-X-02` | `test_rg_x_02` | `tests/test_analysis.c` | Failure table correctness and graph immutability | F1 | All 13 rows match predictions; graph restored byte-identical |
| `RG-X-03` | `test_rg_x_03` | `tests/test_fixture_ref.c` | Exact F1 reference counters verification | F1 | List slots=26, Matrix slots=144, DFS pushes=14, pops=14, stale pops=2 |
| `RG-X-04` | `test_rg_x_04` | `tests/test_graph.c` | Self-loop edge rejection (`u == v`) | F1 | Returns `RG_ERR_SELF_LOOP`; graph unchanged |
| `RG-X-05` | `test_rg_x_05` | `tests/test_graph.c` | Duplicate undirected edge rejection (`(1,0)` after `(0,1)`) | F1 | Returns `RG_ERR_DUPLICATE_EDGE`; graph unchanged |
| `RG-X-06` | `test_rg_x_06` | `tests/test_graph.c` | Graph creation parameter validation | — | $n < 1$ or negative capacity returns `RG_ERR_INVALID_PARAM` |
| `RG-X-07` | `test_rg_x_07` | `tests/test_analysis.c` | Multi-component & single-vertex component counts | F2, F3 | F2 has exactly 4 components; F3 has exactly 1 component |
| `RG-X-08` | `test_rg_x_08` | `tests/test_traverse.c` | Iterative DFS preorder vs Recursive DFS preorder | F1, F2, F4, F5 | Exact preorder agreement across all benchmark topologies |
| `RG-X-09` | `test_rg_x_09` | `tests/test_traverse.c` | Slot-count complexity check (matrix vs list) | F1 | Matrix scans 144 slots ($V^2$) vs List scans 26 slots ($2E$) |
| `RG-X-10` | `test_rg_x_10_differential_oracle` | `tests/test_random.c` | Independent differential testing oracle (300 random graphs) | Random G(V,p) | 67,574 assertions; 0 mismatches against relaxation and label propagation oracles |
| `RG-X-11` | `test_rg_x_11_wasm_bridge` | `wasm/test_bridge.c` (`make test-bridge`) | WebAssembly C11 bridge contract & JSON serialization | F1 & Campus | Full validation of init, reset, BFS/DFS metrics, edge toggles, failure table, and SVG across F1 and MUJ Campus fixtures |
| `RG-X-12` | `test_rg_x_12_campus` | `tests/test_fixture_ref.c` | MUJ Campus Demo topology, direct Main Gate link, VIP Gate (13), direct Dome/Main connections, and e6/e12 failure scenarios | MUJ Campus Demo | 14 vertices, 21 edges, 1-hop 0->3, 2-hop baseline 0->6, 4-hop e6 detour, 1-hop VIP connections |

---

## 3. Untested Items & Exclusions

In accordance with verification honesty requirements, the following items are explicitly not verified through automated testing on this host:
1. **Dynamic Sanitizer Testing (ASan / UBSan) on Native Windows**: **UNTESTED** on Windows MinGW host due to missing toolchain runtime libraries `libasan`/`libubsan`. (Verified portably in official `gcc:14` Linux container).
2. **SVG XML Schema Conformance (`xmllint`)**: **UNTESTED** via `xmllint` due to tool absence.
3. **Browser Graphical Rendering of Native File-Exported SVGs**: **UNTESTED** programmatically via automated headless tools (RouteGuard native engine is an ISO C11 terminal program). Interactive SVG rendering in the WebAssembly frontend was fully verified in a live browser session (`docs/evidence/frontend_manual_checklist.md`).

