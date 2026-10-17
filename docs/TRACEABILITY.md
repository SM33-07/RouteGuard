# RouteGuard — Requirement Traceability Matrix (SRS v0.2)

This matrix traces every functional and non-functional requirement from **SRS v0.2 Section 3, Section 4, and Section 10** to its concrete ISO C11 code implementation, associated automated tests, and verification status.

## Status Legend
- **Tested**: Implemented and verified via automated test runs (`make test` / `build.ps1 -Target test`).
- **Implemented**: Implemented in C11 source/documentation and verified via interactive demo or toolchain checks.
- **Deferred**: Deliberately postponed outside core engine (e.g. optional web UI / WebAssembly presentation layer).

---

## 1. Functional Requirements Matrix

| Requirement ID | SRS v0.2 Text & Specification | Primary Code Location | Associated Test IDs | Status |
|:---|:---|:---|:---|:---:|
| `FR-001` — `FR-008` | **Graph Model & Validation:** Undirected unweighted graph; vertex labels & coords; canonical edge array; dual matrix & sorted list; input validation; allocation wrappers | `src/rg_graph.c`<br>`src/rg_alloc.c` | `RG-GR-01`<br>`RG-GR-02`<br>`RG-ERR-01`<br>`RG-MEM-01`<br>`RG-X-04`<br>`RG-X-05`<br>`RG-X-06` | **Tested** |
| `FR-009` — `FR-013` | **BFS Traversal & Routing:** Layered BFS; mark visited on enqueue; FIFO queue; distance & predecessor tracking; minimum-hop route reconstruction; tie-breaking policy | `src/rg_traverse.c`<br>`src/rg_queue.c` | `RG-BFS-01`<br>`RG-BFS-02`<br>`RG-BFS-03`<br>`RG-BFS-04`<br>`RG-REP-01` | **Tested** |
| `FR-014` — `FR-017` | **DFS Traversal & Reachability:** Iterative DFS with explicit stack; mark visited on pop; descending neighbor push; stale pop accounting; reachability check; recursive cross-check | `src/rg_traverse.c`<br>`src/rg_stack.c` | `RG-DFS-01`<br>`RG-DFS-02`<br>`RG-X-08` | **Tested** |
| `FR-018` — `FR-021` | **Connectivity & Components:** Connected components via repeated traversal; isolated vertex support; component ID tagging | `src/rg_analysis.c` | `RG-COMP-01`<br>`RG-X-07` | **Tested** |
| `FR-022` — `FR-025` | **Failure Simulation & Resilience:** Temporary edge disable/enable; $O(1)$ state mutation; before-and-after impact summary; repeated BFS failure table; graph state restoration | `src/rg_graph.c`<br>`src/rg_analysis.c` | `RG-FAIL-01`<br>`RG-FAIL-02`<br>`RG-FAIL-03`<br>`RG-X-01`<br>`RG-X-02`<br>`RG-REP-02` | **Tested** |
| `FR-026` — `FR-028` | **Complexity Metrics & Counters:** Operation counters: vertices processed, neighbor slots examined, enqueues/dequeues, pushes/pops, stale pops, inactive skips; determinism | `src/rg_traverse.c` | `RG-MET-01`<br>`RG-X-03`<br>`RG-X-09` | **Tested** |
| `FR-029` | **Graph view:** *"If a web interface is delivered, it shall display labelled vertices and connections with distinct visual states for active and disabled edges."* | `frontend/src/App.tsx`<br>`wasm/rg_wasm_bridge.c` | Browser subagent session<br>`docs/evidence/frontend_manual_checklist.md` | **Tested** |
| `FR-030` | **Source/destination controls:** *"The interface shall allow selection of valid source and destination vertices."* | `frontend/src/App.tsx` | Browser subagent session<br>`docs/evidence/frontend_manual_checklist.md` | **Tested** |
| `FR-031` | **Algorithm controls:** *"The interface shall provide actions for BFS, DFS, minimum-hop route, connectivity/component analysis, and graph reset."* | `frontend/src/App.tsx`<br>`wasm/rg_wasm_bridge.c` | Browser subagent session<br>`docs/evidence/frontend_manual_checklist.md` | **Tested** |
| `FR-032` | **Traversal/route display:** *"Results shall show traversal order or the ordered sequence of vertices on the selected route."* | `frontend/src/App.tsx` | Browser subagent session<br>`docs/evidence/frontend_manual_checklist.md` | **Tested** |
| `FR-033` | **Accessible state cues:** *"Active/disabled edges and reached/unreached vertices shall not be distinguished by colour alone; labels, patterns, icons, or text must also convey state."* | `frontend/src/App.tsx`<br>`src/rg_svg.c` | Browser subagent session<br>`docs/evidence/frontend_manual_checklist.md` | **Tested** |
| `AC-010` | **UI Integration:** *"UI (if implemented): source/destination controls, traversal actions, edge toggling, reset, and result/error states work with the C engine."* | `wasm/rg_wasm_bridge.c`<br>`frontend/src/wasm/bridge.ts` | `RG-X-11`<br>Browser subagent session | **Tested** |
| `FR-034` | **Errors:** *"Invalid selections, missing vertices, and failed operations shall produce readable messages without crashing the program."* | `src/main.c`<br>`src/rg_graph.c`<br>`src/rg_traverse.c`<br>`wasm/rg_wasm_bridge.c` | `RG-ERR-01`<br>`RG-ERR-02`<br>`cli_malformed_input.txt` | **Tested** |
| `FR-035` | **Synthetic-data disclosure:** *"The app and documentation shall identify the default network as synthetic and educational."* | `src/main.c`<br>`src/rg_svg.c`<br>`frontend/src/App.tsx`<br>`README.md`<br>`docs/DESIGN.md` | `demo-check`<br>Source inspection | **Implemented** |
| `FR-036` | **Deterministic fixture:** *"The repository shall contain a small graph fixture whose traversal orders, reachability, and shortest-hop routes can be checked by hand."* | `src/rg_fixture.c`<br>`docs/FIXTURES.md` | `RG-BFS-01`<br>`RG-DFS-01`<br>`RG-ROUTE-01`<br>`RG-X-03` | **Tested** |
| `FR-037` | **Unit tests:** *"Graph creation, both representations, BFS, DFS, route reconstruction, component counting, reset, and edge disable/enable shall have automated tests."* | `tests/test_*.c` (27 tests)<br>`wasm/test_bridge.c` (RG-X-11) | Automated suite (27/27 pass)<br>`make test-bridge` (1/1 pass) | **Tested** |
| `FR-038` | **Cross-representation tests:** *"Equivalent matrix/list graph states shall agree on reachability, shortest-hop distance, and component count for all deterministic fixtures."* | `tests/test_traverse.c`<br>`tests/test_analysis.c`<br>`tests/test_random.c` | `RG-REP-01`<br>`RG-REP-02`<br>`RG-COMP-01`<br>`RG-X-06`<br>`RG-X-07`<br>`RG-X-10` | **Tested** |
| `FR-039` | **Regression tests:** *"A single documented command or script shall run the core test suite from a clean build."* | `Makefile` (`make test`)<br>`build.ps1` (`-Target test`) | `docs/evidence/test.txt`<br>`docs/evidence/test_O0.txt`<br>`docs/evidence/test_O2.txt` | **Tested** |
| `FR-040` | **Accurate README:** *"The README shall state the C11 requirement, commands actually tested, supported features, and the optional nature/status of the web/WASM interface."* | `README.md` | Direct inspection | **Implemented** |

---

## 2. Non-Functional & Academic Boundary Requirements

| Requirement ID | Specification | Implementation Strategy | Evidence | Status |
|:---|:---|:---|:---|:---:|
| `NFR-001` | Pure ISO C11 | Compiled with `-std=c11 -Wall -Wextra -pedantic -Wshadow` | `docs/evidence/env.txt` | **Tested** |
| `NFR-002` | Academic Scope Boundary | Exclusively topics through CSC2101 Lecture 37; no Dijkstra, no heaps, no low-link algorithms | Source inspection | **Tested** |
| `NFR-003` | Memory Leak Freedom | Every allocation tracked via `rg_alloc.c`; `rg_alloc_active_count() == 0` asserted; verified under ASan in Linux container | `RG-MEM-01`<br>`sanitizer_linux_container.txt` | **Tested** |
| `NFR-004` | Deterministic Output | Strict tie-breaking (ascending neighbor discovery); byte-identical demo runs | `demo-check` (3,702 bytes) | **Tested** |
| `NFR-005` | Synthetic Data Integrity | Explicit educational disclaimers in CLI, demo header, frontend banner, and SVG | `main.c`, `rg_svg.c`, `App.tsx` | **Implemented** |
| `NFR-006` | Portable Toolchain | Compiler selection precedence (`RG_GCC_DIR` -> `PATH`); OS-aware `Makefile` (Windows NT & POSIX) | `Makefile`, `build.ps1`, `container_build.txt` | **Tested** |

---

## 3. Untested Items & Explicit Exclusions

In accordance with strict verification honesty rules:
1. **ASan & UBSan Dynamic Sanitizers on Native Windows**: **UNTESTED** on native Windows MinGW GCC host due to missing toolchain runtime libraries `libasan`/`libubsan` (linking failure exit code 1). Verified portably inside official Linux container (`gcc:14`, `docs/evidence/sanitizer_linux_container.txt`).
2. **SVG XML Validation via `xmllint`**: **UNTESTED** due to absence of `xmllint` executable on host system.
3. **Browser Visual Rendering of Native File-Exported SVGs**: **UNTESTED** programmatically via automated headless tools (RouteGuard native engine is an ISO C11 terminal program). Interactive SVG rendering in the WebAssembly frontend was fully verified in a live browser session (`docs/evidence/frontend_verification.txt`).
