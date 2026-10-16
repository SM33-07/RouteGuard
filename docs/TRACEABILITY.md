# RouteGuard — Requirement Traceability Matrix (SRS v0.2)

This matrix traces every functional and non-functional requirement from **SRS v0.2 Section 3 & Section 12** to its concrete ISO C11 code implementation, associated automated tests, and verification status.

## Status Legend
- **Tested**: Implemented and verified via automated test runs in this session.
- **Implemented**: Implemented in C11 source and verified via interactive demo.
- **Deferred**: Deliberately postponed outside core engine (e.g. optional web UI / WebAssembly).

---

## 1. Functional Requirements Matrix

| Requirement Group | SRS Requirement IDs | Description Summary | Primary Code Location | Associated Test IDs | Verification Status |
|:---|:---|:---|:---|:---|:---:|
| **Graph Model & Validation** | `FR-001` — `FR-008` | Undirected unweighted graph; vertex labels & coords; canonical edge array; dual matrix & sorted list; input validation; allocation wrappers | `src/rg_graph.c`<br>`src/rg_alloc.c` | `RG-GR-01`<br>`RG-GR-02`<br>`RG-ERR-01`<br>`RG-MEM-01`<br>`RG-X-04`<br>`RG-X-05`<br>`RG-X-06` | **Tested** |
| **BFS Traversal & Routing** | `FR-009` — `FR-013` | Layered BFS; mark visited on enqueue; FIFO queue; distance & predecessor tracking; minimum-hop route reconstruction; tie-breaking policy | `src/rg_traverse.c`<br>`src/rg_queue.c` | `RG-BFS-01`<br>`RG-BFS-02`<br>`RG-BFS-03`<br>`RG-BFS-04`<br>`RG-REP-01` | **Tested** |
| **DFS Traversal & Reachability** | `FR-014` — `FR-017` | Iterative DFS with explicit stack; mark visited on pop; descending neighbor push; stale pop accounting; reachability check; recursive cross-check | `src/rg_traverse.c`<br>`src/rg_stack.c` | `RG-DFS-01`<br>`RG-DFS-02`<br>`RG-X-08` | **Tested** |
| **Connectivity & Components** | `FR-018` — `FR-021` | Connected components via repeated traversal; isolated vertex support; component ID tagging | `src/rg_analysis.c` | `RG-COMP-01`<br>`RG-X-07` | **Tested** |
| **Failure Simulation & Resilience** | `FR-022` — `FR-025` | Temporary edge disable/enable; O(1) state mutation; before-and-after impact summary; repeated BFS failure table; graph state restoration | `src/rg_graph.c`<br>`src/rg_analysis.c` | `RG-FAIL-01`<br>`RG-FAIL-02`<br>`RG-FAIL-03`<br>`RG-X-01`<br>`RG-X-02`<br>`RG-REP-02` | **Tested** |
| **Complexity Metrics & Counters** | `FR-026` — `FR-028` | Operation counters: vertices processed, neighbor slots examined, enqueues/dequeues, pushes/pops, stale pops, inactive skips; determinism | `src/rg_traverse.c` | `RG-MET-01`<br>`RG-X-03`<br>`RG-X-09` | **Tested** |
| **CLI & Scripted Demo** | `FR-029` — `FR-032` | Safe text interface (`fgets`+`strtol`); menu workflow; scripted deterministic `--demo` scenario; zero-diff check | `src/main.c` | Makefile `demo-check`<br>`build.ps1 -Target demo-check` | **Tested** |
| **SVG Visual Topology Export** | `FR-033` — `FR-034` | Pure C11 `fprintf` SVG generation; dashed closed edges; unreached node styling; numbered route hops; educational disclaimer | `src/rg_svg.c` | Interactive CLI Option 13<br>`--demo` mode export | **Tested** |
| **Web Presentation Layer** | `FR-035` — `FR-040` | Minimal React/TypeScript web interface; WebAssembly compilation | — | — | **Deferred** |

---

## 2. Non-Functional & Academic Boundary Requirements

| Requirement ID | Specification | Implementation Strategy | Evidence | Status |
|:---|:---|:---|:---|:---:|
| `NFR-001` | Pure ISO C11 | Compiled with `-std=c11 -Wall -Wextra -pedantic -Wshadow` | `docs/evidence/env.txt` | **Tested** |
| `NFR-002` | Academic Scope Boundary | Exclusively topics through CSC2101 Lecture 37; no Dijkstra, no heaps, no low-link algorithms | Source inspection | **Tested** |
| `NFR-003` | Memory Leak Freedom | Every allocation tracked via `rg_alloc.c`; `rg_alloc_active_count() == 0` asserted | `RG-MEM-01` | **Tested** |
| `NFR-004` | Deterministic Output | Strict tie-breaking (ascending neighbor discovery); byte-identical demo runs | `demo-check` (3702 bytes) | **Tested** |
| `NFR-005` | Synthetic Data Integrity | Explicit educational disclaimers in CLI, demo header, and SVG banner | `main.c`, `rg_svg.c` | **Tested** |
| `NFR-006` | Portable Toolchain | Compiler selection precedence (`RG_GCC_DIR` -> `PATH`); no hard-coded machine paths | `Makefile`, `build.ps1` | **Tested** |

---

## 3. Untested Items & Explicit Exclusions

1. **ASan & UBSan Dynamic Sanitizers**: **UNTESTED** on native Windows MinGW GCC host due to missing toolchain runtime libraries `libasan`/`libubsan`.
2. **SVG XML Validation via `xmllint`**: **UNTESTED** due to tool absence.
3. **Browser Visual Rendering of SVGs**: **UNTESTED** programmatically (RouteGuard is an ISO C11 terminal application).
4. **Web Presentation / WebAssembly (`FR-035` - `FR-040`)**: **UNTESTED** (scope deferred).

