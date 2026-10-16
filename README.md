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

RouteGuard requires an ISO C11 compliant compiler and standard build tools on Windows:

- **C Compiler**: GCC 15.2.0 (MSYS2 UCRT64) or compatible C11 compiler.
- **Build Drivers**: GNU Make (`mingw32-make.exe` 4.4.1) or PowerShell 7 (`pwsh`).
- **Compiler Precedence**:
  1. `RG_GCC_DIR` environment variable (if specified).
  2. `gcc` discoverable on `PATH`.
  3. Actionable error if neither is available.

---

## 4. Build, Test, and Execution Instructions

### A. Environment Setup for Machines Where GCC / Make are NOT on PATH
If `gcc` and `mingw32-make` are installed (e.g. via MSYS2) but not registered in your Windows user/system `PATH`, set `RG_GCC_DIR` and prepend it to your active session:
```powershell
# Set path to the directory containing gcc.exe and mingw32-make.exe
$env:RG_GCC_DIR = "C:\msys64\ucrt64\bin"
$env:PATH = "$env:RG_GCC_DIR;$env:PATH"
```

### B. Using GNU Make (`mingw32-make`)
```powershell
# Standard build (creates bin/routeguard.exe and bin/rg_tests.exe)
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

### C. Using Native PowerShell Driver (`build.ps1`)
```powershell
# Standard build
.\build.ps1 -Target all

# Strict build with -Werror
.\build.ps1 -Target strict

# Run automated tests with configurable optimization (-O0 or -O2 via -OptLevel or RG_OPT)
.\build.ps1 -OptLevel -O0 -Target test
.\build.ps1 -OptLevel -O2 -Target test

# Alternatively using the RG_OPT environment variable:
$env:RG_OPT = "-O0"; .\build.ps1 -Target test
$env:RG_OPT = "-O2"; .\build.ps1 -Target test

# Run deterministic demonstration
.\build.ps1 -Target demo

# Run demo determinism check (byte-for-byte binary diff)
.\build.ps1 -Target demo-check

# Clean build artifacts
.\build.ps1 -Target clean
```

---

## 5. Verification & Test Results

All 26 automated tests have been implemented and executed, strictly tracing to **SRS v0.2 Section 10**:

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

========================================================
TEST RESULTS: 26 Total | 26 Passed | 0 Failed
========================================================
```

- **Demo Determinism**: Verified byte-identical across consecutive executions (3,702 bytes).
- **Optimization Levels Tested**: Passed 26/26 under both `-O0` and `-O2`.
- **Memory Safety**: `rg_alloc_active_count() == 0` verified upon test completion; allocation fault injection verified.

---

## 6. Untested Items & Environment Limitations

In accordance with strict verification honesty rules:
1. **Memory Safety via Sanitizers (ASan/UBSan/LSan)**:
   - **Native Windows MinGW Host**: Marked **UNTESTED** on native Windows due to missing `libasan`/`libubsan` runtimes (linking exit code 1; documented in `docs/evidence/env.txt`).
   - **Linux Container Verification (Official GCC 14.4.0)**: Executed under `-std=c11 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all` inside an official Docker Linux container (`gcc:14`, digest `sha256:6f37e44a9b41181de5dd97366ec732652f5d895a72b5549c64694efed50a97e8`). Verified deliberate-fault detection (heap overflow, signed overflow, and memory leak) and executed the full test suite (26/26 pass) and demo cleanly with zero sanitizer errors and zero leaks. **Note: This checks the portable ISO C11 logic, NOT Windows-specific behaviour.** Full raw log is recorded in `docs/evidence/sanitizer_linux_container.txt`.
2. **SVG XML Validation (`xmllint`)**: Marked **UNTESTED** via `xmllint` because `xmllint` is not installed on the host environment. (SVG output is generated deterministically via strict ISO C `fprintf`).
3. **Browser Visual Rendering**: Marked **UNTESTED** programmatically (RouteGuard is a terminal application; headless browser automation is outside C11 scope).
4. **Web Presentation / WebAssembly**: Marked **UNTESTED** / deferred academic scope.

---

## 7. Repository Layout

```
RouteGuard/
├── AGENTS.md                  # Strict agent instructions & hard scope boundaries
├── README.md                  # Project overview, build instructions, and status
├── Makefile                   # GNU Make build driver
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
│   └── test_fixture_ref.c     # Benchmark metrics and reference tests
├── docs/                      # Authoritative Documentation
│   ├── DESIGN.md              # Invariants, algorithms, pseudocode, and complexity
│   ├── FIXTURES.md            # Hand-traced step tables and closure matrices
│   ├── TEST_PLAN.md           # SRS Section 10 test contract mapping
│   ├── TRACEABILITY.md        # Requirements traceability matrix
│   ├── reference/             # Authoritative SRS v0.2 and project prompts
│   └── evidence/              # Raw probe and test run logs (env.txt, test_O0.txt, test_O2.txt)
└── out/                       # Generated SVG files and demo text outputs (gitignored)
```
