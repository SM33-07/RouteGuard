# RouteGuard

**Author:** `<STUDENT NAME>`  
**Registration Number:** `<REG NO>`  
**Course:** CSC2101 (Data Structures and Algorithms) / CSC2130 (DSA Lab)  
**Implementation Standard:** Pure ISO C11 (Standard Library only)  
**Academic Scope Boundary:** Strictly bounded through CSC2101 Lecture 37  

> **Educational Disclaimer:** All graph topologies, vertex names, and walkway configurations in RouteGuard are **purely synthetic** (educational campus model). Route length corresponds strictly to the number of edges (hops), not physical distance or travel time. This software is not intended for real-world navigation, municipal transit planning, or emergency dispatch.

---

## 1. Project Overview & Scope Evolution

RouteGuard is an interactive graph-based routing and network connectivity simulator designed to evaluate shortest-path discovery, network reachability, connected components, and single-edge pathway-failure scenarios.

*Version Note (v0.1 to v0.2 migration):*  
Earlier project notes described an aspirational C++17 / Next.js / WebAssembly implementation (SRS v0.1). In accordance with academic guidelines for CSC2101/CSC2130, RouteGuard v0.2 is implemented as a **pure ISO C11** application using only the C11 standard library (`<stdio.h>`, `<stdlib.h>`, `<stdbool.h>`, `<string.h>`, `<errno.h>`). Out-of-scope concepts (Dijkstra, A*, priority queues/heaps, minimum spanning trees, disjoint-set/union-find, and low-link bridge algorithms) are strictly excluded.

### SRS v0.2 Requirement Status Summary
- **Core Engine & Data Structures (`FR-001` — `FR-028`)**: Fully implemented in ISO C11 and verified by automated unit tests.
- **Web Presentation Layer (`FR-029` — `FR-033`, `AC-010`)**: **Deferred**. The SRS specifies web-based presentation conditionally (*"If a web interface is delivered..."* in FR-029). For the academic C11 scope, visualization and user controls are delivered natively via the terminal CLI (`src/main.c`) and pure C11 SVG export (`src/rg_svg.c`).
- **Resilience, Testing & Documentation (`FR-034` — `FR-040`)**: Fully implemented and verified:
  - `FR-034` (Errors): Validated against malformed input, invalid selections, and buffer overflows without crashing.
  - `FR-035` (Synthetic disclosure): Explicit educational disclosures in CLI, SVG banner, and docs.
  - `FR-036` (Deterministic fixtures): Deterministic fixtures F1..F5 with hand-traced tables in `docs/FIXTURES.md`.
  - `FR-037` (Unit tests): 27 automated unit tests covering all engine components.
  - `FR-038` (Cross-representation tests): Full parity verified between matrix and list across fixtures and random graphs.
  - `FR-039` (Regression tests): Single documented `make test` / `build.ps1 -Target test` harness.
  - `FR-040` (Accurate README): Complete documentation of verified commands, tools, and deferred scope.

---

## 2. Supported Data Structures & Algorithms

- **Graph Representations**: Dual representation derived from a canonical edge array:
  - Adjacency Matrix ($O(V^2)$): $V \times V$ matrix storing edge IDs or `-1`.
  - Adjacency List ($O(V + E)$): Singly linked list nodes sorted in strictly ascending order of neighbor ID.
  - Active Edge State: Maintained exclusively in `edges[id].active`, enabling $O(1)$ edge toggle operations without representation drift.
- **Breadth-First Search (BFS)**:
  - Fixed-capacity FIFO queue (`rg_queue.c`).
  - Visited status marked **on enqueue** to guarantee each vertex enters the queue at most once.
  - Generates level-order predecessor tree for minimum-hop route reconstruction.
- **Depth-First Search (DFS)**:
  - Explicit fixed-capacity LIFO stack (`rg_stack.c`, capacity $2E + 1$).
  - Visited status marked **on pop**, pushing unvisited neighbors in **descending ID order** so the smallest ID is popped first.
  - Reentrant duplicate pops are tracked as **stale pops**.
  - Cross-validated against a recursive reference implementation (`dfs_list_recursive`).
- **Connected Components**:
  - Derived via repeated BFS traversals across unvisited vertices, handling isolated vertices.
- **Single-Failure Impact Analysis**:
  - Brute-force repeated BFS ($O(E \cdot (V + E))$) evaluating reachability, hop counts, and component changes upon closing each walkway.

---

## 3. Toolchain & Prerequisites

RouteGuard supports both native Windows (MSYS2 / UCRT64) and POSIX / Linux environments:

- **C Compiler**: GCC 15.2.0 (MSYS2 UCRT64 on Windows), GCC 14.2.0+ (Linux), or any conforming C11 compiler.
- **Build Drivers**: GNU Make (`mingw32-make.exe` on Windows, `make` on Linux) or PowerShell 7 (`pwsh`).
- **Compiler Precedence**:
  1. `RG_GCC_DIR` environment variable (if specified).
  2. `gcc` discoverable on `PATH`.
  3. Actionable error if neither is available.
- **OS-Aware Makefile**: The `Makefile` dynamically inspects `ifeq ($(OS),Windows_NT)` and configures native Windows commands (`cmd.exe /c`, `del`, `fc /b`) or standard POSIX commands (`mkdir -p`, `rm -rf`, `cmp -s`).

---

## 4. Build, Test, and Execution Instructions

### A. Environment Setup for Machines Where GCC / Make are NOT on PATH (Windows)
If `gcc` and `mingw32-make` are installed (e.g. via MSYS2) but not registered in your Windows user/system `PATH`, set `RG_GCC_DIR` and prepend it to your active session:
```powershell
# Set path to the directory containing gcc.exe and mingw32-make.exe
$env:RG_GCC_DIR = "C:\msys64\ucrt64\bin"
$env:PATH = "$env:RG_GCC_DIR;$env:PATH"
```

### B. Using GNU Make (`mingw32-make` on Windows, `make` on Linux/macOS)
```powershell
# Standard build (creates bin/routeguard[.exe] and bin/rg_tests[.exe])
mingw32-make all

# Strict build (enforces -Werror, warning-free)
mingw32-make strict

# Run automated test suite (default optimization: -O2)
mingw32-make test

# Configurable optimization level via RG_OPT (e.g. -O0 or -O2; OPT is supported as an alias)
mingw32-make test RG_OPT=-O0
mingw32-make test RG_OPT=-O2
mingw32-make test OPT=-O2

# Run interactive CLI
mingw32-make demo

# Verify deterministic demo output (byte-identical across runs)
mingw32-make demo-check

# Clean build artifacts
mingw32-make clean
```

### C. Using Native PowerShell Driver (`build.ps1` on Windows)
```powershell
# Standard build
.\build.ps1 -Target all

# Strict build with -Werror
.\build.ps1 -Target strict

# Run automated tests with configurable optimization (-O0 or -O2 via -OptLevel or RG_OPT)
.\build.ps1 -OptLevel -O0 -Target test
.\build.ps1 -OptLevel -O2 -Target test

# Run deterministic demonstration
.\build.ps1 -Target demo

# Run demo determinism check (byte-for-byte binary diff)
.\build.ps1 -Target demo-check

# Clean build artifacts
.\build.ps1 -Target clean
```

### D. Direct GCC Compilation on Linux / macOS (Without Make)
If building directly with `gcc` on Linux or macOS without GNU Make:
```bash
mkdir -p bin obj
gcc -std=c11 -Wall -Wextra -pedantic -Wshadow -O2 -Iinclude \
    src/rg_alloc.c src/rg_graph.c src/rg_queue.c src/rg_stack.c \
    src/rg_traverse.c src/rg_analysis.c src/rg_fixture.c src/rg_svg.c \
    src/main.c -o bin/routeguard

gcc -std=c11 -Wall -Wextra -pedantic -Wshadow -O2 -Iinclude \
    src/rg_alloc.c src/rg_graph.c src/rg_queue.c src/rg_stack.c \
    src/rg_traverse.c src/rg_analysis.c src/rg_fixture.c src/rg_svg.c \
    tests/test_main.c tests/test_graph.c tests/test_traverse.c \
    tests/test_analysis.c tests/test_fixture_ref.c tests/test_random.c \
    -o bin/rg_tests

./bin/rg_tests
```

---

## 5. Verification & Test Results

### Automated Test Suite (27/27 Passed)
All 27 automated tests strictly trace to **SRS v0.2 Section 10**:

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

- **Demo Determinism**: Verified byte-identical across consecutive executions (3,702 bytes).
- **Optimization Levels Tested**: Passed 27/27 under both `-O0` and `-O2`.
- **Memory Safety**: `rg_alloc_active_count() == 0` verified upon test completion; dynamic allocation fault injection verified.

### Independent Differential Testing Oracle (`RG-X-10`)
To independently verify BFS shortest-path distance, predecessor validity, iterative-vs-recursive DFS equivalence, and component counting without external dependencies:
- **Oracle Method**: Implemented in `tests/test_random.c` using multi-pass unweighted relaxation (Bellman-Ford principle, $V-1$ rounds) and iterative label propagation.
- **Workload**: Evaluated across 300 pseudo-random connected and disconnected graphs generated via a fixed-seed Linear Congruential Generator (`seed = 20261017`, $V \in [1, 20]$, edge probabilities $p \in [0.0, 0.7]$).
- **Results**: 67,574 individual state comparisons; **0 mismatches** (`docs/evidence/differential_testing.txt`).

### Mutation Testing (100% Kill Rate)
Evaluated across 10 distinct fault injection mutants in an isolated temporary clone:
- **Mutants Evaluated**: 10
- **Mutants Killed**: 10 (100.0% kill rate)
- **Results**: Every injected mutation (omitted marks, reversed adjacency lists, omitted source in route reconstruction, ignored active flags, skipped isolated vertices, off-by-one matrix loops, and removed duplicate/self-loop checks) was detected and killed by the test suite (`docs/evidence/mutation.txt`).

### Malformed Input & CLI Fuzz Testing under Sanitizers
Tested in an official `gcc:14` container under `-fsanitize=address,undefined`:
- Fed with 5,000-character line buffers, non-numeric strings, negative integers, empty lines, EOF termination, 64-bit integer overflow (`99999999999999999999`), and out-of-range menu options.
- The interactive CLI handled every malformed input cleanly, emitted user-friendly error messages, and terminated with exit code 0 without any sanitizer faults or crashes (`docs/evidence/cli_malformed_input.txt`).

---

## 6. Per-Commit Standalone Build & Test Verification

Every commit in the repository history was checked out and evaluated independently with both `make all` AND `make test` (raw log recorded in `docs/evidence/commit_builds.txt`):

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

## 7. Untested Items & Environment Limitations

In accordance with strict verification honesty rules:
1. **Memory Safety via Sanitizers (ASan/UBSan/LSan)**:
   - **Native Windows MinGW Host**: Marked **UNTESTED** on native Windows due to missing `libasan`/`libubsan` runtimes (linking exit code 1; documented in `docs/evidence/env.txt`).
   - **Linux Container Verification (Official GCC 14.4.0)**: Executed under `-std=c11 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all` inside an official Docker Linux container (`gcc:14`, digest `sha256:6f37e44a9b41181de5dd97366ec732652f5d895a72b5549c64694efed50a97e8`). Verified deliberate-fault detection (heap overflow, signed overflow, and memory leak) and executed the full test suite (27/27 pass) and demo cleanly with zero sanitizer errors and zero leaks (`docs/evidence/sanitizer_linux_container.txt`).
2. **SVG XML Validation (`xmllint`)**: Marked **UNTESTED** via `xmllint` because `xmllint` is not installed on the host environment. (SVG output is generated deterministically via strict ISO C `fprintf`).
3. **Browser Visual Rendering**: Marked **UNTESTED** programmatically (RouteGuard is a terminal application; headless browser automation is outside C11 scope).
4. **Web Presentation / WebAssembly (`FR-029` — `FR-033`, `AC-010`)**: Marked **UNTESTED** / deferred academic scope.

---

## 8. Repository Layout

```
RouteGuard/
├── AGENTS.md                  # Strict agent instructions & hard scope boundaries
├── README.md                  # Project overview, build instructions, and status
├── Makefile                   # OS-aware GNU Make build driver (Windows NT & POSIX)
├── build.ps1                  # Native PowerShell build driver
├── .gitignore                 # Object, binary, and output directory exclusions
├── include/                   # ISO C11 Public Headers
│   ├── rg_alloc.h             # Memory allocation wrappers and leak tracking
│   ├── rg_graph.h             # Graph data structures and validation
│   ├── rg_queue.h             # FIFO queue for BFS
│   ├── rg_stack.h             # LIFO stack for DFS
│   ├── rg_traverse.h          # BFS, DFS, and route reconstruction
│   ├── rg_analysis.h          # Components and single-failure analysis
│   ├── rg_fixture.h           # Benchmark fixtures F1 through F5
│   └── rg_svg.h               # Deterministic SVG export
├── src/                       # ISO C11 Source Implementations
│   ├── rg_alloc.c             # Allocator wrappers and fault injection hook
│   ├── rg_graph.c             # Dual representation graph engine
│   ├── rg_queue.c             # Fixed-capacity queue implementation
│   ├── rg_stack.c             # Fixed-capacity stack implementation
│   ├── rg_traverse.c          # Traversal routines and metrics tracking
│   ├── rg_analysis.c          # Repeated BFS component & failure analysis
│   ├── rg_fixture.c           # Campus and edge-case benchmark graphs
│   ├── rg_svg.c               # XML-compliant SVG generator
│   └── main.c                 # Menu-driven CLI and deterministic demo runner
├── tests/                     # Automated Test Suite
│   ├── rg_test.h              # Minimal assertion framework
│   ├── test_main.c            # Test runner entry point
│   ├── test_graph.c           # Graph structure and memory tests
│   ├── test_traverse.c        # BFS and DFS traversal tests
│   ├── test_analysis.c        # Resilience and failure analysis tests
│   ├── test_fixture_ref.c     # Benchmark metrics and reference tests
│   └── test_random.c          # Differential testing oracle (300 random graphs)
├── docs/                      # Authoritative Documentation
│   ├── DESIGN.md              # Invariants, algorithms, pseudocode, and complexity
│   ├── FIXTURES.md            # Hand-traced step tables and closure matrices
│   ├── TEST_PLAN.md           # SRS Section 10 test contract mapping
│   ├── TRACEABILITY.md        # Requirements traceability matrix
│   ├── VIVA_NOTES.md          # 25 examiner viva preparation questions & answers
│   ├── reference/             # Authoritative SRS v0.2 and project prompts
│   └── evidence/              # Raw verification logs
│       ├── env.txt            # Host toolchain and probe logs
│       ├── build.txt          # Clean compilation log
│       ├── test.txt           # Test suite output (27/27 pass)
│       ├── test_O0.txt        # Test suite output under -O0
│       ├── test_O2.txt        # Test suite output under -O2
│       ├── commit_builds.txt  # Standalone per-commit build and test logs
│       ├── differential_testing.txt # Random graph oracle output
│       ├── mutation.txt       # 10 mutants evaluated and killed
│       ├── cli_malformed_input.txt # Fuzz input under sanitizer in container
│       ├── sanitizer_linux_container.txt # Full ASan+UBSan run
│       └── SUMMARY.md         # Comprehensive evidence synthesis
└── out/                       # Generated SVG files and demo text outputs (gitignored)
```
