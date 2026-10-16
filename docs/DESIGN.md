# RouteGuard — System Architecture & Design Specification

## 1. Executive Overview & Scope
RouteGuard is an educational, deterministic route discovery and network resilience simulator written strictly in **ISO C11**. It models a synthetic campus walkway network as an **undirected, unweighted graph** bounded to concepts taught through **CSC2101 Lecture 37**.

---

## 2. Data Model & Architecture

### Graph Structure (`rg_graph.h`)
The graph engine maintains dual representations derived from a single canonical edge array:
```c
typedef struct {
    int u;
    int v;
    bool active;
} RgEdge;

typedef struct RgAdjNode {
    int neighbor;
    int edge_id;
    struct RgAdjNode *next;
} RgAdjNode;

typedef struct {
    int n_vertices;
    int n_edges;
    int max_edges;
    char (*labels)[32];
    int *coords_x;
    int *coords_y;
    RgEdge *edges;            /* Canonical edge array: active flag lives ONLY here */
    int *matrix;              /* n * n array storing edge_id (0..n_edges-1) or -1 */
    RgAdjNode **adj_list;     /* Singly linked lists sorted in strictly ascending order */
} RgGraph;
```

### Representation Synchronization Strategy
- Edge active/disabled state lives exclusively in `edges[edge_id].active`.
- Adjacency matrix cells store `edge_id` (or `-1` if absent).
- Adjacency list nodes store `neighbor` and `edge_id`.
- Toggling an edge ($O(1)$) directly alters `edges[edge_id].active`. Both matrix and list algorithms consult this field, preventing state divergence.

---

## 3. SRS Section 4.3 Invariants Mapping

| Invariant # | Invariant Statement | Enforcing Component | Verifying Test ID | Status |
|:---:|:---|:---|:---:|:---:|
| **1** | Every vertex ID is unique and within valid range $[0, n-1]$. | `rg_graph_create`, `rg_add_edge` | `RG-GR-01`, `RG-GR-02`, `RG-X-06` | **VERIFIED** |
| **2** | Every edge endpoint references an existing vertex. | `rg_add_edge` parameter validation | `RG-GR-02` | **VERIFIED** |
| **3** | No self-loop or duplicate undirected edge exists. | `rg_add_edge` matrix lookup & equality check | `RG-X-04`, `RG-X-05` | **VERIFIED** |
| **4** | Graph is undirected: adjacency $u \to v$ implies $v \to u$. | Symmetric matrix entries & dual list insertion | `RG-GR-01` | **VERIFIED** |
| **5** | Matrix and list views represent identical edges and active state. | Canonical `edges[id].active` queried by both | `RG-REP-01`, `RG-REP-02` | **VERIFIED** |
| **6** | BFS marks vertex visited on enqueue; enters queue at most once. | `bfs_matrix`, `bfs_list` enqueue logic | `RG-BFS-01`, `RG-X-03` | **VERIFIED** |
| **7** | DFS terminates deterministically without infinite loops. | `dfs_matrix`, `dfs_list` mark-on-pop logic | `RG-DFS-01`, `RG-X-08` | **VERIFIED** |
| **8** | Reconstructed route connects source to destination via active edges. | `rg_find_route` predecessor walk | `RG-BFS-02`, `RG-FAIL-02` | **VERIFIED** |
| **9** | BFS route minimizes hop count in unweighted graph. | Unweighted BFS level-order properties | `RG-BFS-02`, `RG-FAIL-02` | **VERIFIED** |
| **10** | Toggling an edge does not alter vertex identity or ordering. | In-place boolean flag modification | `RG-FAIL-01`, `RG-FAIL-02` | **VERIFIED** |
| **11** | Reset restores all edges to active state without topology loss. | `rg_graph_reset` iteration | `RG-FAIL-03` | **VERIFIED** |
| **12** | All dynamic memory is released on shutdown and failure paths. | `rg_alloc.c` active counter tracking | `RG-MEM-01` | **VERIFIED** |

---

## 4. Traversal & Analysis Algorithms

### Breadth-First Search (BFS)
- **Data Structure**: Array-based FIFO queue (`rg_queue.c`) with capacity $V$.
- **Visited Policy**: Mark visited **ON ENQUEUE**.
- **Tie-Breaking**: Adjacency list nodes are inserted in sorted ascending order. Matrix scans columns $0 \dots V-1$ ascending.
- **Pseudocode**:
```text
function BFS(graph, src):
    mark src visited; dist[src] = 0; pred[src] = -1
    enqueue(src)
    while queue is not empty:
        u = dequeue()
        append u to visit_order
        for each neighbor v of u (ascending order):
            examine_slot()
            if edge(u, v) is inactive: continue
            if v is not visited:
                mark v visited
                dist[v] = dist[u] + 1
                pred[v] = u
                enqueue(v)
```

### Depth-First Search (DFS)
- **Data Structure**: Explicit array-based LIFO stack (`rg_stack.c`) with capacity $2E + 1$.
- **Visited Policy**: Mark visited **ON POP**.
- **Push Policy**: Push unvisited active neighbors in **descending ID order** so the smallest neighbor ID is on the top of the stack and popped first.
- **Stale Pop Accounting**: If a popped vertex was already marked visited, increment `stale_pops` and continue.
- **Pseudocode**:
```text
function DFS(graph, src):
    push(src)
    while stack is not empty:
        u = pop()
        if u is visited:
            stale_pops++
            continue
        mark u visited
        append u to visit_order
        for each unvisited neighbor v of u (descending order):
            examine_slot()
            if edge(u, v) is inactive: continue
            push(v)
```

### Single-Failure Impact Analysis
- **Complexity**: $O(E \cdot (V + E))$ on adjacency lists.
- **Implementation**: Brute-force repeated BFS. For each edge $e \in 0 \dots E-1$, temporarily set `edges[e].active = false`, execute BFS from source, record reachability and component count, and restore `edges[e].active = true`.
- **Integrity**: Before-and-after graph state is asserted to be byte-identical.

---

## 5. Metric Definitions (`rg_traverse.h`)
- `vertices_processed`: Number of vertices dequeued by BFS or popped-and-not-stale by DFS.
- `neighbor_slots_examined`: Matrix: Every inspected column $j \in 0 \dots V-1$ for each processed vertex ($V \times \text{processed}$). List: Every stored adjacency node inspected.
- `edges_skipped_inactive`: Slots holding an edge ID whose edge is currently disabled.
- `queue_enqueues` / `queue_dequeues`: Counter of queue operations in BFS.
- `stack_pushes` / `stack_pops`: Counter of stack operations in DFS.
- `stale_pops`: Count of DFS stack pops where the vertex had already been visited.
- `elapsed_ms`: Optional execution time; strictly informational and never asserted in tests.

---

## 6. Asymptotic Complexity Reference

| Operation | Adjacency Matrix (Implemented) | Adjacency List (Implemented) | Empirical Benchmark on F1 (Baseline) |
|---|---|---|---|
| Space Complexity | $O(V^2)$ | $O(V + E)$ | Matrix: 144 ints; List: 12 head pointers + 26 nodes |
| Neighbor Scan (one vertex) | $O(V)$ | $O(\text{degree}(v))$ | Matrix: 12 scans; List: 1 to 4 scans |
| BFS Traversal | $O(V^2)$ | $O(V + E)$ | Matrix: 144 slots; List: 26 slots |
| DFS Traversal | $O(V^2)$ | $O(V + E)$ | Matrix: 144 slots; List: 26 slots |
| Shortest-Hop Route | $O(V^2)$ | $O(V + E)$ | Identical 8-hop route reconstructed |
| Connected Components | $O(V^2)$ | $O(V + E)$ | Matrix & List both identify 1 component |
| Edge Disable / Enable | $O(1)$ | $O(1)$ | Direct toggle in canonical edge array |
| Single-Failure Table | $O(E \cdot V^2)$ | $O(E \cdot (V + E))$ | 13 iterations: List BFS evaluated per edge |

---

## 7. Architectural Decisions & Discrepancy Log

### Decisions
1. **Sorted Adjacency List Insertion**: Insert new neighbors into `adj_list[u]` in strictly ascending order of neighbor ID via sorted pointer traversal. This ensures matrix and list traversals discover neighbors in identical order without sorting at traversal time.
2. **Canonical Active Flag**: Active state resides exclusively in `edges[id].active`. Matrix and list structures only store `edge_id`.
3. **Dual Build Driver Architecture**: Native Windows PowerShell script (`build.ps1`) and standard `Makefile` for `mingw32-make`, both observing `RG_GCC_DIR` precedence.

### Conflicts & Discrepancies
- **Prompt vs SRS Test-ID Meaning**: Prompt A originally listed test IDs with different assignments (e.g. `RG-FAIL-01` as generic toggle idempotency). In accordance with the project instructions, **SRS v0.2 Section 10 is authoritative**. `RG-FAIL-01` strictly tests disabling critical connecting edge `e9`, `RG-FAIL-02` tests redundant edge `e5`, and `RG-FAIL-03` tests re-enable and reset. Generic toggle tests are assigned to `RG-X-01` and `RG-X-02`.

---

## 8. Build Configuration & Untested Items

### Optimization Knob (`RG_OPT` / `OPT`)
- **Authoritative Variable**: `RG_OPT` controls the C compiler optimization flag (e.g., `-O0`, `-O2`).
- **Backward-Compatible Alias**: `OPT` is supported in both `Makefile` and `build.ps1` as a direct alias for `RG_OPT`.
- **Precedence**: `RG_OPT` > `OPT` > Default (`-O2`).
- **Usage**:
  - `mingw32-make test RG_OPT=-O0`
  - `mingw32-make test OPT=-O2`
  - `.\build.ps1 -OptLevel -O0 -Target test`
  - `$env:RG_OPT = "-O0"; .\build.ps1 -Target test`

### Shell Robustness
- In `Makefile`, `SHELL := $(COMSPEC)` (or `cmd.exe`) is enforced on Windows NT. This ensures `mingw32-make` recipes execute correctly even if MSYS2 `usr\bin` (containing `sh.exe`) is on `PATH`.

### Explicit List of UNTESTED Items
In strict compliance with academic and verification honesty standards:
1. **Dynamic Memory Sanitizers (AddressSanitizer & UndefinedBehaviorSanitizer)**:
   - **Status**: **UNTESTED** on native Windows MinGW GCC host.
   - **Reason**: Toolchain lacks `libasan` and `libubsan` static/shared runtime libraries (exit code 1 on linking).
   - **Mitigation**: Internal allocation tracking wrapper (`rg_alloc.c`) asserts active allocation balance `rg_alloc_active_count() == 0` across all test suites, and dynamic fault injection validates out-of-memory graceful recovery.
2. **XML Schema / Well-Formedness Validation via `xmllint`**:
   - **Status**: **UNTESTED** via `xmllint`.
   - **Reason**: The `xmllint` executable is not installed on the host environment.
   - **Mitigation**: SVG generation uses deterministic, strict `fprintf` format strings matching the exact SVG 1.1 XML specification.
3. **Browser Visual Rendering of SVG Outputs**:
   - **Status**: **UNTESTED** programmatically.
   - **Reason**: RouteGuard is a purely terminal-based ISO C11 educational application; headless browser automation is outside the C11 academic scope.
4. **Web Presentation / WebAssembly**:
   - **Status**: **UNTESTED** (scope deferred per Section 0 rules).

