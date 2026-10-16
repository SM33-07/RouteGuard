# RouteGuard — Comprehensive Verification Evidence Summary

**Standard:** ISO C11 Standard Library (`-std=c11 -Wall -Wextra -pedantic -Wshadow`)  
**Academic Scope:** CSC2101 / CSC2130 (Through Lecture 37)  
**Date of Verification:** 2026-10-17  
**Verification Host:** Windows 11 Enterprise (UCRT64 MinGW GCC 15.2.0, GNU Make 4.4.1)  
**Verification Container:** Official Linux Docker Image `gcc:14` (GCC 14.4.0, Debian)  

---

## 1. Toolchain & Environment Matrix

| Component | Host Environment (Windows NT) | Container Environment (POSIX / Linux) |
|:---|:---|:---|
| **Operating System** | Windows 11 (build 26100) | Linux 6.6.87.2-microsoft-standard-WSL2 (Debian) |
| **C Compiler** | `gcc.exe` (Rev13, Built by MSYS2 project) 15.2.0 | `gcc` (Debian 14.4.0-2) 14.4.0 |
| **Make Driver** | GNU Make 4.4.1 (`mingw32-make.exe`) | GNU Make 4.4.1 (`make`) |
| **Shell Driver** | PowerShell 7.5.4 / `cmd.exe` | GNU bash 5.2.32 |
| **C Standard** | ISO C11 (`-std=c11`) | ISO C11 (`-std=c11`) |
| **Compiler Flags** | `-std=c11 -Wall -Wextra -pedantic -Wshadow -O2` | `-std=c11 -Wall -Wextra -pedantic -Wshadow -O2` |
| **Sanitizers** | Marked **UNTESTED** on host (missing `libasan`/`libubsan`) | `-fsanitize=address,undefined -fno-sanitize-recover=all` |
| **Evidence Path** | `docs/evidence/env.txt` | `docs/evidence/sanitizer_linux_container.txt` |

---

## 2. Compilation & Strict Build Verification

All targets build clean and warning-free under strict compiler settings (`-Werror`):

| Build Target | Exact Command | Exit Code | Result | Evidence File |
|:---|:---|:---:|:---:|:---|
| **Host Standard Build** | `mingw32-make clean && mingw32-make all` | `0` | **PASS** (Zero warnings, binaries created) | `docs/evidence/build.txt` |
| **Host Strict Build** | `mingw32-make clean && mingw32-make strict` | `0` | **PASS** (Zero warnings with `-Werror`) | `docs/evidence/build.txt` |
| **Host PS1 Driver** | `.\build.ps1 -Target strict` | `0` | **PASS** (Warning-free via PowerShell) | `docs/evidence/env.txt` |
| **Container Build** | `make clean && make all` (inside `gcc:14`) | `0` | **PASS** (Byte-compatible POSIX branch) | `docs/evidence/container_build.txt` |
| **Container ASan+UBSan** | `make clean && make test RG_SAN=1` (in container) | `0` | **PASS** (Zero sanitizer warnings or leaks) | `docs/evidence/sanitizer_linux_container.txt` |

---

## 3. Automated Test Suite Execution (27/27 Tests)

The full test suite was executed across multiple optimization levels on both Windows and Linux container environments:

```
========================================================
RouteGuard Automated Test Suite (SRS v0.2 Traceable)
ISO C11 Scope - CSC2101 Lecture 37 Academic Boundary
========================================================

Running RG-GR-01             ... [ PASS ]
Running RG-GR-02             ... [ PASS ]
Running RG-ERR-01            ... [ PASS ]
Running RG-MEM-01            ... [ PASS ]
Running RG-X-04              ... [ PASS ]
Running RG-X-05              ... [ PASS ]
Running RG-X-06              ... [ PASS ]
Running RG-BFS-01            ... [ PASS ]
Running RG-BFS-02            ... [ PASS ]
Running RG-BFS-03            ... [ PASS ]
Running RG-BFS-04            ... [ PASS ]
Running RG-DFS-01            ... [ PASS ]
Running RG-DFS-02            ... [ PASS ]
Running RG-REP-01            ... [ PASS ]
Running RG-REP-02            ... [ PASS ]
Running RG-X-08              ... [ PASS ]
Running RG-X-09              ... [ PASS ]
Running RG-COMP-01           ... [ PASS ]
Running RG-FAIL-01           ... [ PASS ]
Running RG-FAIL-02           ... [ PASS ]
Running RG-FAIL-03           ... [ PASS ]
Running RG-X-01              ... [ PASS ]
Running RG-X-02              ... [ PASS ]
Running RG-X-07              ... [ PASS ]
Running RG-MET-01            ... [ PASS ]
Running RG-X-03              ... [ PASS ]
Running RG-X-10              ... [ PASS ]

========================================================
TEST RESULTS: 27 Total | 27 Passed | 0 Failed
========================================================
```

### Execution Across Optimization Configurations:
- **Default Optimization (`-O2`)**: `mingw32-make test` -> **27 / 27 Passed** (`docs/evidence/test.txt`, `docs/evidence/test_O2.txt`).
- **Unoptimized Debug (`-O0`)**: `mingw32-make test RG_OPT=-O0` -> **27 / 27 Passed** (`docs/evidence/test_O0.txt`).
- **PowerShell Driver**: `.\build.ps1 -OptLevel -O0 -Target test` and `-O2` -> **27 / 27 Passed**.

---

## 4. Independent Differential Testing Oracle (`RG-X-10`)

To verify the core BFS, DFS, and connectivity algorithms independently without external libraries or comparing against unverified implementations:
- **Oracle Algorithm**: Multi-pass unweighted relaxation (Bellman-Ford principle running $V-1$ iterations over all edges) for shortest paths, and iterative label propagation until convergence for connected components (`tests/test_random.c`).
- **PRNG Configuration**: Deterministic Linear Congruential Generator (`seed = 20261017`).
- **Test Set**: 300 pseudo-random connected and disconnected graphs ($V \in [1, 20]$, edge probability $p \in [0.0, 0.7]$).
- **Assertions Evaluated**: 67,574 individual state comparisons.
- **Result**: **0 mismatches** (`docs/evidence/differential_testing.txt`).

---

## 5. Mutation Testing Analysis (10/10 Killed — 100% Kill Rate)

In an isolated temporary clone (`$env:TEMP\rg_mutants`), 10 subtle algorithmic bugs were systematically injected one at a time and evaluated against the test suite (`docs/evidence/mutation.txt`):

| ID | Status | Injected Mutation Description | Catching / Killing Tests |
|:---:|:---:|:---|:---|
| **1** | **KILLED** | BFS marks visited on dequeue instead of on enqueue | Build / runtime failure |
| **2** | **KILLED** | Adjacency list built with head insertion (reverses order) | `RG-BFS-01`, `RG-BFS-02`, `RG-DFS-01`, `RG-REP-01`, `RG-REP-02`, `RG-FAIL-02` |
| **3** | **KILLED** | Route reconstruction omits the source vertex | `RG-BFS-02`, `RG-FAIL-02` |
| **4** | **KILLED** | `disable_edge` stops honoring active flag in list traversal | `RG-REP-02`, `RG-FAIL-01`, `RG-FAIL-02`, `RG-X-02`, `RG-X-10` |
| **5** | **KILLED** | DFS marks visited on push instead of on pop | `RG-DFS-01`, `RG-X-08`, `RG-X-03`, `RG-X-10` |
| **6** | **KILLED** | Component counting skips isolated vertices | `RG-X-07`, `RG-X-10` |
| **7** | **KILLED** | `rg_graph_reset` fails to re-activate edges | `RG-FAIL-03` |
| **8** | **KILLED** | Duplicate-edge check removed | `RG-X-05`, `RG-X-10` |
| **9** | **KILLED** | Self-loop check removed | `RG-X-04` |
| **10** | **KILLED** | Matrix scan loop runs to $n-1$ instead of $n$ (off-by-one) | `RG-REP-01`, `RG-REP-02`, `RG-X-09`, `RG-X-03`, `RG-X-10` |

---

## 6. Malformed Input Fuzzing under ASan/UBSan Container

Tested in the official `gcc:14` container under `-fsanitize=address,undefined` (`docs/evidence/cli_malformed_input.txt`):
- **Inputs Tested**:
  1. 5,000-character line buffers (buffer overflow / line length probe).
  2. Non-numeric strings (`"abcdef"`, `"invalid_choice"`).
  3. Negative values (`-1`, `-999`).
  4. 64-bit integer overflow (`99999999999999999999`).
  5. Empty lines (repeated newlines `\n`).
  6. Out-of-range menu selections (`99`).
  7. Premature `EOF` (`Ctrl+D` simulation).
- **Result**: The CLI emitted descriptive, readable validation messages for each malformed entry, prompted again or terminated cleanly on EOF, and exited with **returncode 0** with **zero sanitizer errors, zero memory leaks, and zero crashes**.

---

## 7. Per-Commit Standalone Build & Test Verification

Every commit in the repository history was checked out and evaluated independently with both `make all` AND `make test` (`docs/evidence/commit_builds.txt`):

| Commit | Summary | `make all` | `make test` | Standalone Status | Notes |
|:---|:---|:---:|:---:|:---:|:---|
| `e65dd59` | Initial commit | N/A | N/A | Pre-skeleton | Repository initialization (`.gitignore`, `LICENSE`, `README.md`; no build scripts or C sources yet) |
| `7c8871e` | Project skeleton, build files, AGENTS.md | Fail | Fail | Intermediate | Staged commit: Makefile targets `obj/rg_alloc.o` before source is added |
| `6d53cd2` | Graph model, validation, alloc wrappers | Fail | Fail | Intermediate | Staged commit: Makefile targets `obj/rg_queue.o` |
| `70f01d6` | Queue, stack, BFS/DFS, route reconstruction | Fail | Fail | Intermediate | Staged commit: Makefile targets `obj/rg_analysis.o` |
| `ba66531` | Components, impact summary, failure table | Fail | Fail | Intermediate | Staged commit: Makefile targets `obj/rg_svg.o` |
| `adb0361` | CLI, demo mode, SVG export | Fail | Fail | Intermediate | `routeguard.exe` builds cleanly; test harness not yet added |
| `5a1b8f8` | Test suite and test harness | **PASS** | **PASS** | **Functional** | First fully self-contained build: 26/26 tests pass |
| `7282be3` | Verification gates, fixtures, traceable tests | **PASS** | **PASS** | **Functional** | 26/26 tests pass; strict -Werror clean |
| `01df170` | Reference metrics, DFS cross-check, test docs | **PASS** | **PASS** | **Functional** | 26/26 tests pass; metrics and recursive DFS added |
| `94161be` | Design, traceability, and evidence documentation | **PASS** | **PASS** | **Functional** | 26/26 tests pass; documentation updated |
| `a00a5ec` | Mutation-derived tests and differential oracle | **PASS** | **PASS** | **Functional** | 27/27 tests pass (includes `RG-X-10` differential oracle) |

---

## 8. Fresh Clone & Path Robustness Verification

1. **Path-Absent Toolchain Test (`RG_GCC_DIR`)**:
   - Tested by simulating an environment where `gcc` and `mingw32-make` are absent from `PATH`.
   - Executing with `$env:RG_GCC_DIR = "C:\msys64\ucrt64\bin"` correctly resolved all tools, built binaries, and passed the test suite (`docs/evidence/env.txt`).
2. **MSYS2 `usr\bin` Path Conflict Test**:
   - Tested with `C:\msys64\usr\bin` on `PATH` (which places `sh.exe` in the path).
   - Because `Makefile` enforces `SHELL := $(COMSPEC)` on `Windows_NT`, `demo-check` invoked `fc /b` through `cmd.exe` without path collision or syntax error.

---

## 9. Benchmark Fixture F1 Reference Verification

Baseline reference values for Educational Campus Benchmark (Fixture F1: 12 vertices, 13 edges) verified across test suite runs (`tests/test_fixture_ref.c`):

| Metric | Adjacency Matrix | Adjacency List | Notes |
|:---|:---:|:---:|:---|
| **Space Required** | 144 integers ($V^2$) | 12 heads + 26 nodes ($V + 2E$) | Exact storage footprint |
| **BFS Vertices Processed** | 12 | 12 | All reachable from node 0 |
| **BFS Slots Examined** | 144 | 26 | $V^2$ vs $2E$ |
| **BFS Enqueues / Dequeues** | 12 / 12 | 12 / 12 | Exactly 1 enqueue per vertex |
| **BFS Shortest Route (0 -> 10)** | 8 hops | 8 hops | `0 -> 1 -> 3 -> 4 -> 6 -> 7 -> 8 -> 9 -> 10` |
| **DFS Vertices Processed** | 12 | 12 | Full preorder traversal |
| **DFS Slots Examined** | 144 | 26 | $V^2$ vs $2E$ |
| **DFS Stack Pushes / Pops** | 14 / 14 | 14 / 14 | 12 initial + 2 re-entrant pushes |
| **DFS Stale Pops** | 2 | 2 | Nodes 1 and 2 popped second time |
| **Connected Components** | 1 | 1 | Single connected graph |
| **Disabling Critical Edge e9 (6-7)** | 2 components | 2 components | Disconnects node 10 (route unreachable) |

---

## 10. Explicit List of UNTESTED Items & Justifications

In strict accordance with academic verification honesty:
1. **ASan & UBSan Dynamic Sanitizers on Native Windows MinGW Host**:
   - **Status**: **UNTESTED** on native Windows MinGW host.
   - **Reason**: The host MSYS2 UCRT64 GCC toolchain lacks the static/dynamic `libasan` and `libubsan` runtimes for the Windows target (linking fails with exit code 1; documented in `docs/evidence/env.txt`).
   - **Mitigation**: Internal allocation tracking wrapper (`rg_alloc.c`) asserts active allocation balance `rg_alloc_active_count() == 0` across all test suites, and dynamic fault injection validates out-of-memory graceful recovery. Furthermore, full ASan and UBSan runs were verified inside an official GCC 14.4.0 Linux container (`docs/evidence/sanitizer_linux_container.txt`).
2. **SVG XML Validation via `xmllint`**:
   - **Status**: **UNTESTED** via `xmllint`.
   - **Reason**: The `xmllint` executable is not installed on the host environment.
   - **Mitigation**: SVG generation uses deterministic, strict `fprintf` format strings matching the exact SVG 1.1 XML specification.
3. **Browser Visual Rendering of SVG Outputs**:
   - **Status**: **UNTESTED** programmatically.
   - **Reason**: RouteGuard is a purely terminal-based ISO C11 educational application; headless browser automation is outside the C11 academic scope.
4. **Web Presentation Layer / WebAssembly (`FR-029` — `FR-033`, `AC-010`)**:
   - **Status**: **UNTESTED** / **Deferred** (optional web UI outside C11 scope).
