# RouteGuard

**Author:** `<STUDENT NAME>`  
**Registration Number:** `<REG NO>`  
**Course:** CSC2101 (Data Structures and Algorithms) / CSC2130 (DSA Lab)  
**Implementation Standard:** Pure ISO C11 (Standard Library only) + WebAssembly (Emscripten)  
**Academic Scope Boundary:** Strictly bounded through CSC2101 Lecture 37  

> **Educational Disclaimer:** All graph topologies, vertex names, and walkway configurations in RouteGuard are **purely synthetic** (educational campus model). Route length corresponds strictly to the number of edges (hops), not physical distance or travel time. This software is not intended for real-world navigation, municipal transit planning, or emergency dispatch.

---

## 1. Project Overview & Scope Evolution

RouteGuard is an interactive graph-based routing and network connectivity simulator designed to evaluate shortest-path discovery, network reachability, connected components, and single-edge pathway-failure scenarios.

*Architecture Overview:*  
- **Core Engine:** Written strictly in **pure ISO C11** using only standard library headers (`<stdio.h>`, `<stdlib.h>`, `<stdbool.h>`, `<string.h>`, `<errno.h>`). It implements dual graph representations (Adjacency Matrix and Adjacency List), Breadth-First Search (BFS), Depth-First Search (DFS), predecessor route reconstruction, connected components, and brute-force single-failure analysis. Out-of-scope concepts (Dijkstra, A*, priority queues/heaps, minimum spanning trees, disjoint-set/union-find, and low-link bridge algorithms) are strictly excluded.
- **Native CLI:** Interactive menu-driven console interface and deterministic batch test runner (`routeguard.exe`).
- **WebAssembly Presentation Layer:** The authoritative C11 engine compiled to WebAssembly via Emscripten and exposed through a safe C bridge (`src/rg_wasm_bridge.c`).
- **Interactive Browser Frontend:** A polished dark-mode network resilience dashboard built with React 19, TypeScript, and Vite in `frontend/`, rendering interactive SVG network topologies and live analytics directly from the compiled C WebAssembly engine.

### SRS v0.2 Requirement Status Summary
- **Core Engine & Data Structures (`FR-001` — `FR-028`)**: Fully implemented in ISO C11 and verified by automated unit tests.
- **Web Presentation Layer (`FR-029` — `FR-033`, `AC-010`)**: **Implemented & Tested**. Delivered as an interactive WebAssembly-powered browser interface (`frontend/`) backed by the authoritative C11 engine:
  - `FR-029` (Graph view): Labelled vertices and connections with distinct visual states for active and disabled edges.
  - `FR-030` (Source/destination controls): Interactive dropdown and canvas selection for valid source and destination nodes.
  - `FR-031` (Algorithm controls): Instant execution of BFS, DFS, minimum-hop routes, components, and global reset.
  - `FR-032` (Traversal/route display): Sequential chip display of traversal orders and numbered route hops.
  - `FR-033` (Accessible state cues): Active/disabled edges and reached/unreached vertices distinguished by patterns, explicit text labels ("CLOSED", "UNREACHED"), and numbered hop badges (never color alone).
  - `AC-010` (UI Integration): Verified end-to-end in browser with zero JavaScript algorithm reimplementation.
- **Resilience, Testing & Documentation (`FR-034` — `FR-040`)**: Fully implemented and verified:
  - `FR-034` (Errors): Validated against malformed input, invalid selections, and buffer overflows without crashing.
  - `FR-035` (Synthetic disclosure): Explicit educational disclosures in CLI, SVG banner, frontend header, and docs.
  - `FR-036` (Deterministic fixtures): Deterministic fixtures F1..F5 with hand-traced tables in `docs/FIXTURES.md`.
  - `FR-037` (Unit tests): 28 automated unit tests covering all engine components and bridge interfaces.
  - `FR-038` (Cross-representation tests): Full parity verified between matrix and list across fixtures and random graphs.
  - `FR-039` (Regression tests): Single documented `make test` / `build.ps1 -Target test` harness.
  - `FR-040` (Accurate README): Complete documentation of verified commands, tools, and build workflows.

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

- **Native C Compiler**: GCC 15.2.0 (MSYS2 UCRT64 on Windows), GCC 14.2.0+ (Linux), or any conforming C11 compiler.
- **WebAssembly Compiler**: Emscripten `emcc` 6.0.12+ (available via PATH or `RG_EMSDK_DIR`).
- **Frontend Toolchain**: Node.js v24+ and npm 11+.
- **Build Drivers**: GNU Make (`mingw32-make.exe` on Windows, `make` on Linux) or PowerShell 7 (`pwsh`).
- **OS-Aware Makefile**: Dynamically inspects `ifeq ($(OS),Windows_NT)` to select native Windows commands or POSIX commands.

---

## 4. Build, Test, and Execution Instructions

### A. Native C Engine: Build and Test
```powershell
# Standard build (creates bin/routeguard.exe and bin/rg_tests.exe)
mingw32-make all
# or using PowerShell:
.\build.ps1 -Target all

# Strict build (enforces -Werror, warning-free)
mingw32-make strict
# or using PowerShell:
.\build.ps1 -Target strict

# Run automated test suite (28/28 passed)
mingw32-make test
# or using PowerShell:
.\build.ps1 -Target test

# Verify deterministic CLI demo output (byte-identical across runs)
mingw32-make demo-check
# or using PowerShell:
.\build.ps1 -Target demo-check
```

### B. WebAssembly Module Compilation
Compiles `src/` (excluding `main.c`) and `src/rg_wasm_bridge.c` to `frontend/src/wasm/routeguard.js` with TypeScript definitions:
```powershell
# Using PowerShell build driver:
.\build.ps1 -Target wasm

# Or using GNU Make:
mingw32-make wasm
```
*Note:* The build script automatically detects Emscripten on `PATH` or resolves it from `$env:RG_EMSDK_DIR` (defaulting to `C:\emsdk`).

### C. Interactive Frontend: Local Development & Production Build
The frontend is a standalone client-side React + TypeScript application that executes the C11 engine inside the browser using WebAssembly. No backend server or database is required.

```powershell
# 1. Navigate to frontend directory
cd frontend

# 2. Install dependencies
npm install

# 3. Start local development server (opens at http://localhost:5173/)
npm run dev

# 4. Build optimized production bundle
npm run build
```

---

## 5. Verification & Test Results

### Automated Test Suite (28/28 Passed)
All 28 automated tests strictly trace to **SRS v0.2 Section 10**:

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
Running RG-X-11              ... [ PASS ]

========================================================
TEST RESULTS: 28 Total | 28 Passed | 0 Failed
========================================================
```

- **Demo Determinism**: Verified byte-identical across consecutive executions (3,702 bytes).
- **Optimization Levels Tested**: Passed 28/28 under both `-O0` and `-O2`.
- **Memory Safety**: `rg_alloc_active_count() == 0` verified upon test completion; dynamic allocation fault injection verified.

### Independent Differential Testing Oracle (`RG-X-10`)
- **Oracle Method**: Implemented in `tests/test_random.c` using multi-pass unweighted relaxation (Bellman-Ford principle, $V-1$ rounds) and iterative label propagation.
- **Workload**: Evaluated across 300 pseudo-random connected and disconnected graphs generated via a fixed-seed Linear Congruential Generator (`seed = 20261017`).
- **Results**: 67,574 individual state comparisons; **0 mismatches** (`docs/evidence/differential_testing.txt`).

### WebAssembly Bridge Contract Test (`RG-X-11`)
- **Bridge Test**: Implemented in `tests/test_bridge.c` validating native and WebAssembly JSON serialization, BFS/DFS metrics, edge toggling, graph reset, single-failure table generation, and SVG export.
- **Result**: Passed cleanly with zero memory leaks.

### Mutation Testing (100% Kill Rate)
Evaluated across 10 distinct fault injection mutants in an isolated temporary clone:
- **Mutants Evaluated**: 10 | **Mutants Killed**: 10 (100.0% kill rate) (`docs/evidence/mutation.txt`).

### Browser Subagent End-to-End Verification
The frontend was fully evaluated in a real browser session (`http://localhost:5173/`, recorded to `routeguard_frontend_test_1792194315768.webp`):
1. **Baseline F1 Verification**: Route 0 (Main Gate) to 10 (Medical Annex) finds 8-hop route `0 → 1 → 3 → 4 → 6 → 7 → 8 → 9 → 10`.
2. **Complexity Counters**: Adjacency List examines 26 slots; Adjacency Matrix examines 144 slots.
3. **Critical Bridge Cut (`e9`)**: Disconnecting e9 immediately updates canvas to "CLOSED", sets route to "Destination Unreachable", partitions network into 2 components, and reduces reachable count to 8.
4. **Bridge Restoration (`e9`)**: Restoring e9 immediately re-establishes the 8-hop route and restores 1 component.
5. **Redundant Cut (`e5`)**: Disconnecting e5 reroutes traffic via Block B (`0 → 1 → 3 → 5 → 6 → 7 → 8 → 9 → 10`), preserving an 8-hop shortest route.
6. **Failure Table**: 13-row resilience matrix rendered directly from C engine.
7. **Console Status**: Zero runtime errors, zero uncaught exceptions.

---

## 6. Repository Layout

```
RouteGuard/
├── AGENTS.md                  # Strict agent instructions & hard scope boundaries
├── README.md                  # Project overview, build instructions, and status
├── Makefile                   # OS-aware GNU Make build driver (Windows NT & POSIX)
├── build.ps1                  # Native PowerShell build driver
├── build_wasm.ps1             # WebAssembly build driver (Emscripten)
├── .gitignore                 # Object, binary, and output directory exclusions
├── include/                   # ISO C11 Public Headers
│   ├── rg_alloc.h             # Memory allocation wrappers and leak tracking
│   ├── rg_graph.h             # Graph data structures and validation
│   ├── rg_queue.h             # FIFO queue for BFS
│   ├── rg_stack.h             # LIFO stack for DFS
│   ├── rg_traverse.h          # BFS, DFS, and route reconstruction
│   ├── rg_analysis.h          # Components and single-failure analysis
│   ├── rg_fixture.h           # Benchmark fixtures F1 through F5
│   ├── rg_svg.h               # Deterministic SVG export
│   └── rg_wasm_bridge.h       # C11-to-WASM bridge header
├── src/                       # ISO C11 Source Implementations
│   ├── rg_alloc.c             # Allocator wrappers and fault injection hook
│   ├── rg_graph.c             # Dual representation graph engine
│   ├── rg_queue.c             # Fixed-capacity queue implementation
│   ├── rg_stack.c             # Fixed-capacity stack implementation
│   ├── rg_traverse.c          # Traversal routines and metrics tracking
│   ├── rg_analysis.c          # Repeated BFS component & failure analysis
│   ├── rg_fixture.c           # Campus and edge-case benchmark graphs
│   ├── rg_svg.c               # XML-compliant SVG generator
│   ├── rg_wasm_bridge.c       # WebAssembly bridge implementation
│   └── main.c                 # Menu-driven CLI and deterministic demo runner
├── tests/                     # Automated Test Suite
│   ├── rg_test.h              # Minimal assertion framework
│   ├── test_main.c            # Test runner entry point
│   ├── test_graph.c           # Graph structure and memory tests
│   ├── test_traverse.c        # BFS and DFS traversal tests
│   ├── test_analysis.c        # Resilience and failure analysis tests
│   ├── test_fixture_ref.c     # Benchmark metrics and reference tests
│   ├── test_random.c          # Differential testing oracle (300 random graphs)
│   └── test_bridge.c          # WASM bridge contract test
├── frontend/                  # React + TypeScript + Vite Browser Frontend
│   ├── index.html             # Application entry point
│   ├── package.json           # Frontend dependencies and scripts
│   ├── vite.config.ts         # Vite bundler configuration
│   └── src/
│       ├── App.tsx            # Network resilience console UI
│       ├── index.css          # Dark-theme operations console styling
│       ├── main.tsx           # React bootstrap entry point
│       └── wasm/              # Compiled WebAssembly artifacts & TypeScript bridge
│           ├── bridge.ts      # Typed TypeScript wrapper for C engine
│           ├── routeguard.js  # Emscripten WASM module
│           └── routeguard.d.ts # TypeScript definitions
└── docs/                      # Authoritative Documentation
    ├── DESIGN.md              # Invariants, algorithms, pseudocode, and complexity
    ├── FIXTURES.md            # Hand-traced step tables and closure matrices
    ├── TEST_PLAN.md           # SRS Section 10 test contract mapping
    ├── TRACEABILITY.md        # Requirements traceability matrix
    ├── VIVA_NOTES.md          # 25 examiner viva preparation questions & answers
    └── evidence/              # Raw verification logs
        ├── env.txt            # Host toolchain and probe logs
        ├── build.txt          # Clean compilation log
        ├── test.txt           # Test suite output
        ├── test_O0.txt        # Test suite output under -O0
        ├── test_O2.txt        # Test suite output under -O2
        ├── commit_builds.txt  # Standalone per-commit build and test logs
        ├── differential_testing.txt # Random graph oracle output
        ├── mutation.txt       # 10 mutants evaluated and killed
        ├── cli_malformed_input.txt # Fuzz input under sanitizer in container
        ├── sanitizer_linux_container.txt # Full ASan+UBSan run
        ├── frontend_verification.txt # WASM and browser verification log
        └── SUMMARY.md         # Comprehensive evidence synthesis
```
