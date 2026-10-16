# RouteGuard — Oral Examination (Viva Voce) Preparation Guide

This document prepares the candidate for oral defense and technical questioning on the RouteGuard codebase, architectural decisions, ISO C11 standard compliance, academic boundary constraints (CSC2101 through Lecture 37), complexity trade-offs, and empirical verification.

---

### Q1: Why does BFS mark vertices visited on enqueue, whereas DFS marks them on pop?
- **Code Citation**: `src/rg_traverse.c` (`bfs_list`, lines 122–134; `dfs_list`, lines 260–273).
- **Answer**:
  - In **BFS**, marking visited *on enqueue* prevents the same vertex from ever entering the FIFO queue multiple times across different branching paths. This bounds queue capacity strictly to $V$ slots and prevents redundant work.
  - In **iterative DFS**, an explicit stack represents the path of deferred exploration. If a vertex were marked on push, alternative paths discovered deeper in the tree could not push that vertex even if it had not yet been processed, breaking depth-first ordering. Marking *on pop* ensures a vertex is only committed to the traversal sequence when control actually reaches it, matching the behavior of recursive activation frames.

---

### Q2: Why is the explicit DFS stack allocated with capacity $2E + 1$, and what is a "stale pop"?
- **Code Citation**: `src/rg_stack.c` (`rg_stack_create`); `src/rg_traverse.c` (`dfs_list`, line 266); `include/rg_traverse.h` (`stale_pops`).
- **Answer**:
  - Because DFS marks vertices visited *on pop* rather than on push, an unvisited vertex $v$ may be pushed onto the stack by multiple neighbors before it is popped and marked. In an undirected graph, each undirected edge $\{u, v\}$ can contribute at most two directed pushes ($u \to v$ and $v \to u$), plus 1 initial push for the source vertex. The absolute upper bound on the stack depth is therefore $2E + 1$.
  - A **stale pop** occurs when a vertex popped from the stack is found to have `res->visited[u] == true` (because it was popped and processed along an earlier branch). The engine increments `res->metrics.stale_pops` and skips it. On fixture F1, exactly 2 stale pops occur (nodes 1 and 2), verified in test `RG-X-03`.

---

### Q3: Why did we implement single-failure and bridge detection via repeated BFS instead of Tarjan's or Kosaraju's low-link algorithm?
- **Code Citation**: `src/rg_analysis.c` (`rg_single_failure_analysis`, lines 170–215); `AGENTS.md` (Academic Scope Boundary).
- **Answer**:
  - Tarjan's and Kosaraju's bridge/articulation-point algorithms require depth-first low-link values (`low[u] = min(disc[u], ...)`), which is an advanced graph topic covered well beyond CSC2101 Lecture 37.
  - Repeated BFS is the canonical brute-force approach bounded strictly within introductory DSA: for each edge $e \in 0 \dots E-1$, we temporarily deactivate $e$, run a standard BFS traversal ($O(V + E)$), evaluate reachability and component counts, and reactivate $e$. The total time is $O(E \cdot (V + E))$, which runs in sub-millisecond time for our network size and preserves strict academic honesty.

---

### Q4: Why is Dijkstra's algorithm forbidden in RouteGuard, and why is BFS mathematically sufficient for shortest paths here?
- **Code Citation**: `AGENTS.md` Section 1; `src/rg_traverse.c` (`rg_find_route`).
- **Answer**:
  - Dijkstra's algorithm is strictly forbidden by the course syllabus boundary and requires a priority queue / min-heap.
  - RouteGuard models an **unweighted graph** where every active edge represents exactly one hop. In an unweighted graph, Breadth-First Search traverses vertices in non-decreasing order of distance from the source. The first time BFS reaches any vertex $v$, the length of the path from the root of the BFS tree is guaranteed to be the minimum possible number of edges. Dijkstra on an unweighted graph degenerates to BFS with unnecessary $O(\log V)$ heap overhead.

---

### Q5: How does RouteGuard maintain dual representations (Adjacency Matrix and Adjacency List) in sync during runtime edge closures?
- **Code Citation**: `include/rg_graph.h` (`RgGraph`, `RgEdge`); `src/rg_graph.c` (`rg_graph_disable_edge`, `rg_graph_enable_edge`).
- **Answer**:
  - Dual representations are kept synchronized through a **single canonical source of truth**: `graph->edges[edge_id].active`.
  - The adjacency matrix does not store boolean flags; it stores the integer `edge_id` (or `-1` if absent).
  - The adjacency list nodes (`RgAdjNode`) store `neighbor` and `edge_id`.
  - Disabling or enabling an edge ($O(1)$) directly toggles `graph->edges[edge_id].active = false/true`. Both matrix and list traversal routines inspect `graph->edges[edge_id].active` before exploring an edge. Thus, neither representation needs to be reallocated, rebuilt, or mutated, guaranteeing zero state drift.

---

### Q6: Why did we use sorted insertion for adjacency lists rather than sorting neighbors during traversal?
- **Code Citation**: `src/rg_graph.c` (`rg_graph_add_edge`, lines 145–165).
- **Answer**:
  - When an edge $(u, v)$ is added, the new `RgAdjNode` is inserted into `graph->adj_list[u]` using single-pointer sorted insertion in ascending order of `neighbor`.
  - Doing this at graph creation time ($O(\text{deg}(u))$ per edge) ensures that every traversal (BFS or DFS) encounters neighbors in naturally ascending order in $O(1)$ per step, eliminating any runtime sorting overhead or auxiliary buffer allocation during traversal. This guarantees that matrix and list traversals produce identical visit orders.

---

### Q7: Why did we avoid dynamic array resizing (`realloc`) during traversals, choosing fixed-capacity queues and stacks instead?
- **Code Citation**: `src/rg_queue.c` (`rg_queue_create`); `src/rg_stack.c` (`rg_stack_create`).
- **Answer**:
  - Dynamic resizing via `realloc` introduces latency spikes ($O(N)$ copy), potential heap fragmentation, and failure points deep inside traversal loops.
  - Because our graph invariants strictly bound the maximum size of both structures—queue capacity is bounded by $V$ (since vertices enter at most once) and stack capacity is bounded by $2E + 1$ (maximum edge pushes)—we allocate fixed-capacity arrays once at queue/stack creation. This provides $O(1)$ guaranteed push/enqueue and pop/dequeue operations without allocation overhead.

---

### Q8: How is demo determinism achieved across different compiler optimizations, platforms, and runs?
- **Code Citation**: `src/main.c` (`run_demo`); `Makefile` (`demo-check`); `tests/test_random.c`.
- **Answer**:
  - Determinism is achieved by:
    1. Eliminating reliance on memory addresses or pointer ordering.
    2. Enforcing strict tie-breaking: matrix scans ascending column IDs $0 \dots V-1$; adjacency lists store neighbors sorted ascending.
    3. Pushing DFS neighbors in descending order so the smallest neighbor ID is popped first.
    4. Avoiding uninitialized memory via calloc/zero-init wrappers (`rg_calloc`).
    5. Excluding non-deterministic metrics like wall-clock execution time from asserted outputs.
  - Determinism is verified by `make demo-check`, which runs `--demo` twice into separate output files and verifies byte-for-byte identity via `fc /b` (Windows) or `cmp -s` (Linux).

---

### Q9: What was the hardest bug caught by the test suite during development?
- **Code Citation**: `src/rg_traverse.c` (`dfs_list`, lines 274–297); `tests/test_traverse.c` (`RG-DFS-01`, `RG-X-08`).
- **Answer**:
  - Ensuring exact equivalence between iterative and recursive DFS on undirected graphs with cycles.
  - In recursive DFS, calling `dfs_rec(v)` immediately descends into $v$'s smallest neighbor. In iterative DFS, to pop the smallest neighbor first, neighbors must be pushed onto the stack in **reverse (descending) order**.
  - However, in adjacency lists where nodes are stored in ascending order, if you push while iterating forward, the largest neighbor ends up on top. To fix this without violating ISO C11 or using dynamic memory, `dfs_list` buffers active unvisited neighbors into a stack-sized local buffer and pushes them in descending order (`for (int i = count - 1; i >= 0; i--)`). This was validated by test `RG-X-08`, which verifies identical visit orders between iterative and recursive DFS on all fixtures.

---

### Q10: Explain the worst-case time and space complexity of every graph operation in RouteGuard.
- **Code Citation**: `docs/DESIGN.md` Section 6.
- **Answer**:
  - **Adjacency Matrix**: Space $O(V^2)$. Scanning neighbors of one vertex: $O(V)$. BFS: $O(V^2)$. DFS: $O(V^2)$. Components: $O(V^2)$. Single failure table: $O(E \cdot V^2)$.
  - **Adjacency List**: Space $O(V + E)$. Scanning neighbors: $O(\text{deg}(u))$. BFS: $O(V + E)$. DFS: $O(V + E)$ (stack pops $2E+1$). Components: $O(V + E)$. Single failure table: $O(E \cdot (V + E))$.
  - **Edge Toggle**: $O(1)$ for both representations.

---

### Q11: How does RouteGuard prevent memory leaks, and how was memory safety verified?
- **Code Citation**: `src/rg_alloc.c` (`rg_malloc`, `rg_free`, `rg_alloc_active_count`); `tests/test_graph.c` (`RG-MEM-01`).
- **Answer**:
  - Every dynamic allocation flows through `rg_alloc.c`, which tracks active allocation pointers and counters. Every teardown function (`rg_graph_free`, `rg_queue_destroy`, `rg_stack_destroy`, `rg_traverse_result_free`) pairs with its constructor.
  - At the end of every test and CLI session, `rg_alloc_active_count() == 0` is asserted.
  - In addition, the complete test suite and CLI were compiled with `-fsanitize=address,undefined` inside an official GCC 14.4.0 Linux container, verifying zero memory leaks and zero invalid accesses across all execution paths.

---

### Q12: Explain the independent differential testing oracle implemented in `tests/test_random.c` and what it guarantees.
- **Code Citation**: `tests/test_random.c` (`test_rg_x_10_differential_oracle`, `oracle_shortest_paths`, `oracle_connected_components`).
- **Answer**:
  - To verify the core engine without relying on external libraries or comparing against potentially buggy third-party implementations, we wrote an independent reference oracle using completely different algorithmic principles:
    1. Shortest path distance: Multi-pass unweighted edge relaxation (the Bellman-Ford principle, running $V-1$ iterations over all edges) rather than BFS queue traversal.
    2. Connected components: Iterative multi-pass label propagation (each node takes `min(label)` of itself and its neighbors until convergence) rather than BFS tree expansion.
  - We tested this across **300 random graphs** (seed `20261017`, $V \in [1, 20]$, varying edge density). Across 67,574 individual state assertions, **0 mismatches** occurred.

---

### Q13: How does the RouteGuard interactive CLI handle malformed, extreme, or hostile input without crashing?
- **Code Citation**: `src/main.c` (`read_int_choice`, lines 35–65); `docs/evidence/cli_malformed_input.txt`.
- **Answer**:
  - The CLI never uses unsafe functions like `gets()` or naked `scanf("%d")` (which leave newlines in the buffer and cause infinite loops on non-numeric input).
  - All input is read into a fixed 128-byte line buffer via `fgets(buf, sizeof(buf), stdin)`. If the line exceeds the buffer, extra characters are flushed until `\n` or `EOF`.
  - Parsing is done using `strtol()` with `endptr` validation, checking for `ERANGE` overflow, non-numeric strings, negative values, and out-of-range menu selections.
  - Tested under AddressSanitizer with 5,000-character lines, strings, negative numbers, 64-bit overflows (`99999999999999999999`), and immediate EOF; all handled cleanly with readable error messages and exit code 0.

---

### Q14: How does RouteGuard's custom memory allocator wrapper (`rg_alloc.c`) support fault injection testing?
- **Code Citation**: `src/rg_alloc.c` (`rg_alloc_set_fail_countdown`, lines 60–75); `tests/test_graph.c` (`RG-MEM-01`).
- **Answer**:
  - `rg_alloc.c` contains a static variable `fail_countdown`. When set to $k$, the allocator decrements the counter on each allocation and returns `NULL` when $k=0$, simulating an out-of-memory error on the $k$-th allocation.
  - Test `RG-MEM-01` systematically sets `fail_countdown = 1` through `fail_countdown = 7` during `rg_graph_create` and `rg_graph_add_edge`, verifying that partial allocations are cleanly rolled back, no memory is leaked, and `RG_ERR_ALLOC` is returned.

---

### Q15: Why is the DFS neighbor push loop executed in descending order of neighbor IDs?
- **Code Citation**: `src/rg_traverse.c` (`dfs_matrix`, line 214; `dfs_list`, line 293).
- **Answer**:
  - A stack is a Last-In, First-Out (LIFO) structure. The last item pushed is the first item popped.
  - RouteGuard's tie-breaking invariant requires that when multiple unvisited neighbors are available, the neighbor with the **smallest vertex ID** must be explored first.
  - Therefore, pushing neighbors in descending order (largest ID first) places the smallest neighbor ID at the top of the stack, ensuring it is popped and explored first.

---

### Q16: How does RouteGuard handle disconnected components and isolated vertices during traversal and component counting?
- **Code Citation**: `src/rg_analysis.c` (`rg_connected_components_matrix`, `rg_connected_components_list`); `tests/test_analysis.c` (`RG-X-07`).
- **Answer**:
  - `rg_connected_components` iterates over all vertex IDs $0 \dots V-1$. If `!visited[i]`, it increments the component counter, assigns `component_ids[i]`, and initiates a BFS queue traversal from $i$ to mark all reachable vertices.
  - An isolated vertex has degree 0. When the outer loop reaches an isolated vertex, it increments the component counter, marks it visited, discovers no neighbors, and terminates that traversal. The isolated vertex correctly forms its own component of size 1. This is validated on Fixture F2 (which has 4 components including isolated vertex 5) in test `RG-X-07`.

---

### Q17: What is the purpose of the canonical edge array (`edges`), and why does the active flag live exclusively there?
- **Code Citation**: `include/rg_graph.h` (`RgGraph`, `RgEdge`); `src/rg_graph.c` (`rg_graph_create`, `rg_graph_add_edge`).
- **Answer**:
  - In an undirected graph, an edge between $u$ and $v$ appears twice in an adjacency matrix (`matrix[u][v]` and `matrix[v][u]`) and twice in an adjacency list (in $u$'s list and in $v$'s list).
  - If the active boolean flag were duplicated in each representation node, toggling an edge would require multiple mutations across disparate heap locations, risking inconsistent partial state.
  - By giving each edge a unique canonical integer ID $0 \dots E-1$ in `graph->edges`, the active status is stored in exactly one memory location (`graph->edges[edge_id].active`). Adjacency matrix and list cells store only the `edge_id`.

---

### Q18: What is the relationship between the iterative DFS (`dfs_list`) and recursive DFS (`dfs_rec_helper`), and how are they cross-verified?
- **Code Citation**: `src/rg_traverse.c` (`dfs_rec_helper`, lines 304–325); `tests/test_traverse.c` (`test_rg_x_08`, lines 185–225).
- **Answer**:
  - Iterative DFS simulates the runtime call stack using an explicit heap-allocated array stack (`RgStack`). Recursive DFS uses the C compiler's machine call stack.
  - Because iterative DFS marks on pop and pushes unvisited neighbors in descending order, its preorder visitation order is mathematically identical to recursive DFS.
  - Test `RG-X-08` executes both implementations on fixtures F1, F2, F4, and F5, and the differential oracle `RG-X-10` executes both on 300 random graphs, asserting that `res_iter->order[i] == res_rec->order[i]` for all $i$.

---

### Q19: How are minimum-hop routes reconstructed from the BFS predecessor array, and what invariants are verified?
- **Code Citation**: `src/rg_traverse.c` (`rg_reconstruct_route`, lines 370–412).
- **Answer**:
  - During BFS, when an unvisited neighbor $v$ is enqueued from $u$, we record `pred[v] = u`.
  - To reconstruct the route to destination `dst`, `rg_reconstruct_route` starts at `curr = dst` and follows `pred[curr]` backwards until reaching `src` or `-1`.
  - The path vertices are accumulated into a temporary array and reversed in place to produce the path sequence from source to destination.
  - Invariants verified: (1) `path[0] == src`, (2) `path[hop_count] == dst`, (3) every consecutive pair $(path[i], path[i+1])$ represents an active edge, and (4) `hop_count == dist[dst]`.

---

### Q20: What is the mathematical and operational meaning of each tracked metric in `RgMetrics`?
- **Code Citation**: `include/rg_traverse.h` (`RgMetrics`).
- **Answer**:
  - `vertices_processed`: Vertices dequeued by BFS or popped-and-not-stale by DFS (equals reachable vertex count).
  - `neighbor_slots_examined`: Total edge lookup operations. In matrix BFS/DFS, equals $V \times \text{vertices\_processed}$. In list BFS/DFS, equals sum of degrees of processed vertices ($\le 2E$).
  - `edges_skipped_inactive`: Encountered edges whose `active` flag was false.
  - `queue_enqueues` / `queue_dequeues`: FIFO queue push and pop operations (equals `vertices_processed` for BFS).
  - `stack_pushes` / `stack_pops`: Total LIFO stack operations (bounded by $2E + 1$).
  - `stale_pops`: DFS pops where the vertex had already been visited along an earlier path.
  - `elapsed_ms`: Wall-clock execution time (labelled informational only; never used in deterministic assertions).

---

### Q21: What are the 10 mutation tests performed on RouteGuard, and what vulnerabilities would surviving mutants represent?
- **Code Citation**: `docs/evidence/mutation.txt`.
- **Answer**:
  - 10 semantic faults were introduced one at a time into a temporary copy:
    1. Mark visited on dequeue in BFS (would cause exponential redundant queue enqueues).
    2. Head insertion in adjacency list (would reverse tie-breaking order).
    3. Omit source in route reconstruction (would corrupt route integrity).
    4. Ignore active flag in list traversal (would traverse closed pathways).
    5. Mark visited on push in DFS (would omit stale pop accounting and change preorder).
    6. Skip isolated vertices in component analysis (would undercount disconnected networks).
    7. Fail to re-activate edges in reset (would leave network permanently degraded).
    8. Remove duplicate-edge check (would allow multi-graphs, violating MVP invariants).
    9. Remove self-loop check (would cause traversal cycles to self).
    10. Off-by-one matrix scan ($V-1$ instead of $V$) (would miss the highest-indexed vertex).
  - All 10 mutants were caught and **KILLED** by the automated test suite (100% kill rate).

---

### Q22: Why did we choose pure ISO C11 standard library only, avoiding POSIX-specific or platform-specific extensions in the source code?
- **Code Citation**: `AGENTS.md` Section 1.
- **Answer**:
  - Portability and pedagogical purity: ISO C11 standard headers (`<stdio.h>`, `<stdlib.h>`, `<stdbool.h>`, `<string.h>`, `<errno.h>`) compile identically across MSYS2/MinGW on Windows, GCC/Clang on Linux, and Apple Clang on macOS without conditional compilation `#ifdef _WIN32` scattered through the source logic.
  - Relying exclusively on standard library data structures forces explicit, transparent implementation of memory management, queues, stacks, and graph traversals.

---

### Q23: How does RouteGuard generate SVG output deterministically in pure C11 without third-party XML/graphics libraries?
- **Code Citation**: `src/rg_svg.c` (`rg_svg_export`, lines 25–115).
- **Answer**:
  - SVG is an XML-based vector graphics format. Since RouteGuard cannot use third-party libraries, `rg_svg.c` generates valid SVG 1.1 directly using standard `fprintf()` writes to a file stream.
  - Geometric vertex coordinates are mapped to a 900x600 viewBox. Active walkways are rendered as solid lines (`#475569`), disabled walkways as dashed red lines (`#ef4444`, `stroke-dasharray="6,4"`), and reconstructed route hops with numbered circle badges (`#2563eb`).
  - Strict compliance with `FR-033` and `FR-035` ensures distinct visual states without color-alone distinctions and includes an explicit synthetic data disclosure banner.

---

### Q24: What are the explicit UNTESTED items in RouteGuard, and why is verification honesty critical in software engineering?
- **Code Citation**: `README.md` Section 7; `docs/DESIGN.md` Section 8; `docs/TRACEABILITY.md` Section 3.
- **Answer**:
  - RouteGuard documents four explicit UNTESTED items:
    1. Dynamic sanitizers (ASan/UBSan) on native Windows MinGW host (due to missing host runtime libraries).
    2. SVG XML formal validation via `xmllint` (executable not installed on host).
    3. Browser visual rendering of SVG outputs (headless browser is outside C11 scope).
    4. Optional Web presentation layer / WebAssembly (`FR-029` — `FR-033`, `AC-010`) (deferred).
  - Verification honesty is a foundational principle: claiming tests passed without verifiable tool output or misrepresenting intermediate build stages compromises academic and engineering integrity.

---

### Q25: If this project were extended beyond CSC2101 Lecture 37, what algorithms and data structures would be incorporated, and why were they excluded here?
- **Code Citation**: `AGENTS.md` Section 1; `docs/DESIGN.md` Section 1.
- **Answer**:
  - Extensions beyond Lecture 37 would incorporate:
    1. **Weighted Edges & Dijkstra's Algorithm**: Utilizing a binary min-heap / priority queue to model physical distances or walking durations rather than unweighted hop counts.
    2. **Tarjan's Bridge-Finding Algorithm**: Using depth-first search discovery times and low-link values to identify critical walkway bottlenecks in a single $O(V + E)$ pass rather than repeated $O(E \cdot (V + E))$ BFS.
    3. **Disjoint Set Union (DSU / Union-Find)**: For dynamic connectivity queries and Kruskal's Minimum Spanning Tree algorithm.
    4. **Hash Tables**: For $O(1)$ vertex lookup by arbitrary string names rather than integer indices.
  - These were excluded because introductory DSA focuses on mastering fundamental arrays, pointer-based linked lists, stacks, queues, and standard graph traversals before introducing advanced optimizations.
