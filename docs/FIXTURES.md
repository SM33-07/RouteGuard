# RouteGuard — Fixtures and Independent Hand Traces

This document provides complete, independently derived step tables and reference values for fixtures F1 through F5, serving as the ground-truth benchmark for all algorithm verification.

---

## 1. Fixture F1 "Campus" Benchmark

### Graph Topology
- **Vertices (12)**:
  - `0`: Main Gate `(50, 200)`
  - `1`: Admin Block `(180, 100)`
  - `2`: Library `(180, 300)`
  - `3`: Central Plaza `(320, 200)`
  - `4`: Block A `(460, 120)`
  - `5`: Block B `(460, 280)`
  - `6`: Hostel Junction `(600, 200)`
  - `7`: Sports Complex `(720, 200)`
  - `8`: Auditorium `(840, 200)`
  - `9`: Health Centre `(960, 200)`
  - `10`: Medical Annex `(1080, 200)`
  - `11`: Parking `(80, 350)`

- **Edges (13)**:
  - `e0: (0, 1)`
  - `e1: (0, 2)`
  - `e2: (0, 11)`
  - `e3: (1, 3)`
  - `e4: (2, 3)`
  - `e5: (3, 4)`
  - `e6: (3, 5)`
  - `e7: (4, 6)`
  - `e8: (5, 6)`
  - `e9: (6, 7)`
  - `e10: (7, 8)`
  - `e11: (8, 9)`
  - `e12: (9, 10)`

- **Ascending Adjacency List Structure**:
  - `0`: `[1, 2, 11]`
  - `1`: `[0, 3]`
  - `2`: `[0, 3]`
  - `3`: `[1, 2, 4, 5]`
  - `4`: `[3, 6]`
  - `5`: `[3, 6]`
  - `6`: `[4, 5, 7]`
  - `7`: `[6, 8]`
  - `8`: `[7, 9]`
  - `9`: `[8, 10]`
  - `10`: `[9]`
  - `11`: `[0]`

---

### F1 BFS Hand Trace (Source: 0)
- **Policy**: Mark visited on **enqueue**. Neighbor discovery tie-breaking: **strictly ascending neighbor ID**.
- **Step Trace**:

| Step | Dequeued Vertex | Queue State After Processing | Visited Set (On Enqueue) | Predecessor Updates | Cumulative List Slots Examined |
|:---:|:---:|:---:|:---:|:---:|:---:|
| Init | — | `[0]` | `{0}` | `dist[0]=0, pred[0]=-1` | 0 |
| 1 | `0` | `[1, 2, 11]` | `{0, 1, 2, 11}` | `pred[1]=0, pred[2]=0, pred[11]=0` | 3 |
| 2 | `1` | `[2, 11, 3]` | `{0, 1, 2, 11, 3}` | `pred[3]=1` | 5 |
| 3 | `2` | `[11, 3]` | `{0, 1, 2, 11, 3}` | None (0 & 3 already visited) | 7 |
| 4 | `11` | `[3]` | `{0, 1, 2, 11, 3}` | None (0 already visited) | 8 |
| 5 | `3` | `[4, 5]` | `{..., 4, 5}` | `pred[4]=3, pred[5]=3` | 12 |
| 6 | `4` | `[5, 6]` | `{..., 6}` | `pred[6]=4` | 14 |
| 7 | `5` | `[6]` | `{..., 6}` | None (3 & 6 already visited) | 16 |
| 8 | `6` | `[7]` | `{..., 7}` | `pred[7]=6` | 19 |
| 9 | `7` | `[8]` | `{..., 8}` | `pred[8]=7` | 21 |
| 10 | `8` | `[9]` | `{..., 9}` | `pred[9]=8` | 23 |
| 11 | `9` | `[10]` | `{..., 10}` | `pred[10]=9` | 25 |
| 12 | `10` | `[]` | `{0..11}` | None (9 already visited) | 26 |

- **Derived BFS Metrics**:
  - Visit Order: `0, 1, 2, 11, 3, 4, 5, 6, 7, 8, 9, 10`
  - Vertices Processed: `12`
  - Queue Enqueues: `12`
  - Queue Dequeues: `12`
  - Adjacency List Neighbor Slots Examined: `26` (sum of all degrees: $2 \times 13 = 26$)
  - Adjacency Matrix Neighbor Slots Examined: `144` ($12 \times 12$)
  - Route from 0 to 10: Reconstruct via `pred`: $10 \leftarrow 9 \leftarrow 8 \leftarrow 7 \leftarrow 6 \leftarrow 4 \leftarrow 3 \leftarrow 1 \leftarrow 0$.
    Reversed path: `0 -> 1 -> 3 -> 4 -> 6 -> 7 -> 8 -> 9 -> 10` (8 hops).

---

### F1 Iterative DFS Hand Trace (Source: 0)
- **Policy**: Mark visited on **pop**. Push unvisited active neighbors in **descending ID order** so smallest neighbor ID sits at the top of the stack and is popped first.
- **Step Trace**:

| Step | Popped Vertex | Visited Check | Action / Visited Set | Pushed Neighbors (Descending) | Stack State (Top at Right) | Cumulative Pushes | Cumulative Pops |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Init | — | — | Push Start | `0` | `[0]` | 1 | 0 |
| 1 | `0` | New | Visit `0` | `11, 2, 1` | `[11, 2, 1]` | 4 | 1 |
| 2 | `1` | New | Visit `1` | `3` | `[11, 2, 3]` | 5 | 2 |
| 3 | `3` | New | Visit `3` | `5, 4, 2` | `[11, 2, 5, 4, 2]` | 8 | 3 |
| 4 | `2` | New | Visit `2` | None (0, 3 visited) | `[11, 2, 5, 4]` | 8 | 4 |
| 5 | `4` | New | Visit `4` | `6` | `[11, 2, 5, 6]` | 9 | 5 |
| 6 | `6` | New | Visit `6` | `7, 5` | `[11, 2, 5, 7, 5]` | 11 | 6 |
| 7 | `5` | New | Visit `5` | None (3, 6 visited) | `[11, 2, 5, 7]` | 11 | 7 |
| 8 | `7` | New | Visit `7` | `8` | `[11, 2, 5, 8]` | 12 | 8 |
| 9 | `8` | New | Visit `8` | `9` | `[11, 2, 5, 9]` | 13 | 9 |
| 10 | `9` | New | Visit `9` | `10` | `[11, 2, 5, 10]` | 14 | 10 |
| 11 | `10` | New | Visit `10` | None (9 visited) | `[11, 2, 5]` | 14 | 11 |
| 12 | `5` | **STALE** | Discard (already visited) | None | `[11, 2]` | 14 | 12 |
| 13 | `2` | **STALE** | Discard (already visited) | None | `[11]` | 14 | 13 |
| 14 | `11` | New | Visit `11` | None (0 visited) | `[]` | 14 | 14 |

- **Derived DFS Metrics**:
  - Visit Order: `0, 1, 3, 2, 4, 6, 5, 7, 8, 9, 10, 11`
  - Stack Pushes: `14`
  - Stack Pops: `14`
  - Stale Pops: `2` (vertex 5 popped at Step 12, vertex 2 popped at Step 13)
  - Non-stale Vertices Processed: `12`
  - Neighbor Slots Examined: `26`

---

### F1 Single-Edge Failure Analysis Table (Source 0, Destination 10)
- Derived via brute-force repeated BFS.
- Notice: `N/A` indicates destination is disconnected (no path), which is distinct from `0 hops` (source == destination).

| Edge ID | Walkway Endpoints | Description / Impact Analysis | Destination 10 Reachable? | Shortest Hops | Reachable from 0 | Component Count |
|:---:|:---:|:---|:---:|:---:|:---:|:---:|
| `e0` | `(0, 1)` | Redundant path; reroutes via `(0, 2) -> (2, 3)` | **YES** | 8 | 12 | 1 |
| `e1` | `(0, 2)` | Redundant path; primary route via Admin Block unchanged | **YES** | 8 | 12 | 1 |
| `e2` | `(0, 11)` | Walkway to Parking closed; Parking (11) is isolated | **YES** | 8 | 11 | **2** |
| `e3` | `(1, 3)` | Redundant path; reroutes via Library `(0, 2) -> (2, 3)` | **YES** | 8 | 12 | 1 |
| `e4` | `(2, 3)` | Redundant path; primary route via Admin Block unchanged | **YES** | 8 | 12 | 1 |
| `e5` | `(3, 4)` | Walkway via Block A closed; reroutes via Block B `(3, 5)` | **YES** | 8 | 12 | 1 |
| `e6` | `(3, 5)` | Walkway via Block B closed; primary route via Block A unchanged | **YES** | 8 | 12 | 1 |
| `e7` | `(4, 6)` | Walkway from Block A closed; reroutes via Block B `(3, 5) -> (5, 6)` | **YES** | 8 | 12 | 1 |
| `e8` | `(5, 6)` | Walkway from Block B closed; primary route via Block A unchanged | **YES** | 8 | 12 | 1 |
| `e9` | `(6, 7)` | **Critical Bridge**: cuts campus from medical/sports facilities | **NO** | **N/A** | **8** | **2** |
| `e10` | `(7, 8)` | **Critical Bridge**: cuts Sports Complex from Auditorium | **NO** | **N/A** | **9** | **2** |
| `e11` | `(8, 9)` | **Critical Bridge**: cuts Auditorium from Health Centre | **NO** | **N/A** | **10** | **2** |
| `e12` | `(9, 10)` | **Critical Bridge**: Medical Annex (10) isolated | **NO** | **N/A** | **11** | **2** |

---

## 2. Edge-Case Fixtures F2 through F5

### Fixture F2 — Disconnected Components with Isolated Vertices
- **Topology**: $V=7$, $E=4$.
  Edges: `(0, 1)`, `(1, 2)`, `(2, 0)` [Triangle component], `(3, 4)` [2-node chain component].
  Vertices `5` and `6` have no incident edges (degree 0).
- **Independent Component Derivation**:
  - Component 0: `{0, 1, 2}`
  - Component 1: `{3, 4}`
  - Component 2: `{5}`
  - Component 3: `{6}`
  - **Total Components**: `4`
- **Reachability from 0**:
  - Reachable: `0, 1, 2` (size 3)
  - Unreachable: `3, 4, 5, 6`
  - Route $0 \to 5$: `RG_NO_ROUTE` (`hop_count = -1`).

### Fixture F3 — Single Vertex Base Case
- **Topology**: $V=1$, $E=0$.
  Vertex `0` only.
- **Independent Derivation**:
  - Connected Components: `1`
  - Route $0 \to 0$: Path `[0]`, `hop_count = 0` (0 edges, 1 vertex).
  - Traversal Order: `[0]` (1 processed, 0 edges examined).

### Fixture F4 — Linear Chain
- **Topology**: $V=4$, $E=3$.
  Edges: `(0, 1)`, `(1, 2)`, `(2, 3)`.
- **Independent Derivation**:
  - BFS Order from 0: `0, 1, 2, 3`
  - DFS Order from 0: `0, 1, 2, 3`
  - Route $0 \to 3$: `0 -> 1 -> 2 -> 3` (3 hops).
  - Components: `1`.

### Fixture F5 — 4-Cycle Tie-Breaking Benchmark
- **Topology**: $V=4$, $E=4$.
  Cycle edges: `(0, 1)`, `(1, 2)`, `(2, 3)`, `(3, 0)`.
- **Independent Derivation for Route $0 \to 2$**:
  - Two equal-length paths exist: `0 -> 1 -> 2` (2 hops) and `0 -> 3 -> 2` (2 hops).
  - Neighbors of 0 are 1 and 3.
  - Ascending discovery order enqueues 1 before 3.
  - When 1 is processed, 2 is discovered and enqueued with `pred[2] = 1`.
  - When 3 is later processed, 2 is already marked visited.
  - Deterministic route produced: `0 -> 1 -> 2` (2 hops).

---

## 3. Reference Discrepancies
- **Conflicting Values**: None.
- **Evaluation**: All independently calculated metrics, visit orders, stack traces, and failure rows strictly agree with the prompt reference values and SRS requirements.

---

## 4. Fixture MUJ Campus Demo Benchmark

### Synthetic Educational Disclaimer
> **“Illustrative graph model — connections are simplified for DSA demonstration and are not verified pedestrian directions.”**
> No real-world walking times, GPS navigation, municipal transit, or emergency routing suitability is claimed.

### Graph Topology
- **Vertices (12)**:
  - `0`: `Main Gate` (Full: `Main Gate — Primary campus arrival & visitor entry`) `(750, 1260)`
  - `1`: `Dome Building` (Full: `Dome Building — Library, administration, services (Landmark 1)`) `(750, 1090)`
  - `2`: `Grand Stairs` (Full: `Grand Stairs / Central Plaza — Central arrival and circulation hub`) `(750, 980)`
  - `3`: `Academic Block 1` (Full: `Academic Block 1 — Left side of the central stairs (Landmark 2)`) `(470, 910)`
  - `4`: `Academic Block 2` (Full: `Academic Block 2 — Right side of the central stairs (Landmark 3)`) `(980, 910)`
  - `5`: `Old Mess / Food Court` (Full: `Old Mess / Food Court — Dining and student seating (Landmark 4)`) `(740, 740)`
  - `6`: `Academic Block 3` (Full: `Academic Block 3 — Arts, media and specialist spaces (Landmark 6)`) `(440, 580)`
  - `7`: `Lecture Hall Complex` (Full: `Lecture Hall Complex — Large teaching halls (Landmark 5)`) `(800, 580)`
  - `8`: `Workshops / Labs` (Full: `Workshops / Labs — Workshop-side teaching facilities (Landmark 8)`) `(1200, 750)`
  - `9`: `Football Ground` (Full: `Football Ground — Sports grounds (Landmark 7)`) `(1190, 560)`
  - `10`: `Cricket Ground` (Full: `Cricket Ground — Sports grounds (Landmark 7)`) `(1170, 360)`
  - `11`: `Outdoor Courts` (Full: `Outdoor Courts / Turfs — Sports grounds (Landmark 7)`) `(1250, 970)`

- **Edges (15)**:
  - `e0: (0, 1)` — Main Gate <-> Dome Building
  - `e1: (1, 2)` — Dome Building <-> Grand Stairs
  - `e2: (2, 3)` — Grand Stairs <-> Academic Block 1
  - `e3: (2, 4)` — Grand Stairs <-> Academic Block 2
  - `e4: (2, 5)` — Grand Stairs <-> Old Mess / Food Court
  - `e5: (3, 5)` — Academic Block 1 <-> Old Mess / Food Court
  - `e6: (3, 6)` — Academic Block 1 <-> Academic Block 3 (Redundant corridor)
  - `e7: (4, 5)` — Academic Block 2 <-> Old Mess / Food Court
  - `e8: (4, 8)` — Academic Block 2 <-> Workshops / Labs
  - `e9: (5, 7)` — Old Mess / Food Court <-> Lecture Hall Complex
  - `e10: (5, 8)` — Old Mess / Food Court <-> Workshops / Labs
  - `e11: (6, 7)` — Academic Block 3 <-> Lecture Hall Complex
  - `e12: (7, 9)` — Lecture Hall Complex <-> Football Ground (Critical sports bridge)
  - `e13: (8, 11)` — Workshops / Labs <-> Outdoor Courts
  - `e14: (9, 10)` — Football Ground <-> Cricket Ground

- **Ascending Adjacency List Structure**:
  - `0`: `[1]`
  - `1`: `[0, 2]`
  - `2`: `[1, 3, 4, 5]`
  - `3`: `[2, 5, 6]`
  - `4`: `[2, 5, 8]`
  - `5`: `[2, 3, 4, 7, 8]`
  - `6`: `[3, 7]`
  - `7`: `[5, 6, 9]`
  - `8`: `[4, 5, 11]`
  - `9`: `[7, 10]`
  - `10`: `[9]`
  - `11`: `[8]`

---

### Campus BFS Hand Trace (Source: 0)
- **Policy**: Mark visited on **enqueue**. Neighbor discovery tie-breaking: **strictly ascending neighbor ID**.
- **Visit Order**: `0, 1, 2, 3, 4, 5, 6, 8, 7, 11, 9, 10`
- **Derived Counters**:
  - Vertices Processed: `12`
  - Queue Enqueues: `12`
  - Queue Dequeues: `12`
  - Adjacency List Neighbor Slots Examined: `30` ($2 \times 15 = 30$)
  - Adjacency Matrix Neighbor Slots Examined: `144` ($12 \times 12 = 144$)
  - Route from 0 (Main Gate) to 6 (Academic Block 3):
    `0 -> 1 -> 2 -> 3 -> 6` (4 hops).
  - Route from 0 (Main Gate) to 9 (Football Ground):
    `0 -> 1 -> 2 -> 5 -> 7 -> 9` (5 hops).

---

### Campus Failure & Rerouting Scenarios
1. **Redundant Path Rerouting (Disable e6: 3 <-> 6)**:
   - Baseline route from 0 (Main Gate) to 6 (Academic Block 3): `0 -> 1 -> 2 -> 3 -> 6` (4 hops via e0, e1, e2, e6).
   - With `e6` disabled: BFS discovers alternate path via Old Mess and LHC: `0 -> 1 -> 2 -> 5 -> 7 -> 6` (5 hops via e0, e1, e4, e9, e11).
   - Graph remains connected (`component_count = 1`, 12 reachable vertices).
2. **Critical Corridor Disconnection (Disable e12: 7 <-> 9)**:
   - Walkway between Lecture Hall Complex and Football Ground closed.
   - Route from 0 to 9: `UNREACHABLE` (`found = false`, `hop_count = -1`).
   - Connected Components: `2` (Component 1: `{0, 1, 2, 3, 4, 5, 6, 7, 8, 11}` of size 10; Component 2: `{9, 10}` of size 2).
   - List BFS from 0: 10 vertices processed, 27 list slots examined, 1 inactive edge skipped (`e12`).
   - Matrix BFS from 0: 10 vertices processed, 120 matrix slots examined ($10 \times 12$).

