# RouteGuard — Master Prompts for the IDE Agent (C11 scope, SRS v0.2)

Two prompts replace the earlier multi-sprint plan. Paste them into your IDE agent one at a time.

**Before Prompt A**
1. Create `C:\Soham\Coding\GitHub\RouteGuard\docs\reference\` (or let the agent create it) and put `RouteGuard_SRS_v0_2.pdf` and `RouteGuard_Submission_Checklist_Report_Outline_v0_2.pdf` there.
2. Put your name / registration number in the placeholders when you review the README. The agent is told NOT to invent them.

**Between A and B (30 minutes, do not skip):** hand-trace BFS and DFS on the fixture in `docs/FIXTURES.md` yourself. You must be able to do this in the viva, and it is the only check that is independent of the agent.

**After B:** bring `docs/evidence/SUMMARY.md` plus the evidence folder back to the chat. The report PDF and slides get built from real outputs only.

---

## PROMPT A — Build everything (engine, tests, CLI, SVG export, docs)

````
# RouteGuard — Master Build Prompt (single pass)

You are implementing RouteGuard end to end in ONE pass: a Graph-Based Route Discovery and Network Connectivity Simulator written in ISO C11 for a university DSA lab (CSC2130). Work through the stages below in order without stopping for approval between stages. Stop only (a) at the end, or (b) if you find existing user work that conflicts with what you were about to write.

## 0. Hard rules
- Language: ISO C11 only. No C++ source, no C++ compiler flags, no third-party libraries, standard library only.
- Academic boundary: DSA concepts through CSC2101 Lecture 37. Allowed: arrays, singly linked lists, stacks, queues, recursion, adjacency matrix, adjacency list, BFS, DFS, connected components via repeated traversal, complexity analysis. FORBIDDEN: Dijkstra, A*, heaps/priority queues, MST, union-find, bridge/articulation/low-link algorithms, hash tables, trees. Do not add them even as "bonus".
- The graph is UNDIRECTED and UNWEIGHTED. Route length = number of edges (hops), never distance or time.
- Synthetic data only. Never imply suitability for real navigation or emergency dispatch.
- Never modify, move, delete, or copy from any repository named LogWatch.
- Target directory: `C:\Soham\Coding\GitHub\RouteGuard`. If it exists, run `git status` first, never overwrite user work. If it contains earlier docs describing C++17 / Next.js / Dijkstra / WASM (SRS v0.1), move them to `docs/superseded-v0.1/` (do not delete) and mention this in your final report.
- Git: make LOCAL commits only, using the commit plan in Stage 9. Never push, never add or change a remote.
- Honesty: never write "passes", "verified", "tested" or a test count anywhere (README, docs, commit messages, final report) unless it comes from command output produced in this session. Paste real output into `docs/evidence/`. Anything you could not run must be labelled UNTESTED with the reason.
- If `docs/reference/RouteGuard_SRS_v0_2.pdf` exists, treat it as authoritative. If it conflicts with this prompt, do not silently pick one: record the conflict in `docs/DESIGN.md` under "Conflicts" and follow this prompt.
- Do not ask questions. If something is unspecified, choose the simplest option within the boundary, record it in `docs/DESIGN.md` under "Decisions", and continue.

## 1. Toolchain inspection (no installs)
Report actual versions/availability of: OS, shell, git, gcc and/or clang and/or MSVC, make/mingw32-make/cmake, xmllint (optional), and sanitizer support (`-fsanitize=address,undefined`). Choose the build driver you can actually run. Provide a Makefile; if `make` is unavailable on this machine also provide `build.ps1` doing the same compile/test/run steps. Document only commands you ran. Save the inspection output to `docs/evidence/env.txt`.
Compiler flags: `-std=c11 -Wall -Wextra -pedantic -Wshadow` (add `-Werror` to a separate `make strict` target). Build must be warning-free; explain any warning you cannot eliminate.

## 2. Repository layout (create exactly this, adjust only with a recorded reason)
```
RouteGuard/
  AGENTS.md  README.md  Makefile  [build.ps1]  .gitignore
  include/  rg_graph.h rg_queue.h rg_stack.h rg_traverse.h rg_analysis.h rg_fixture.h rg_svg.h rg_alloc.h
  src/      rg_graph.c rg_queue.c rg_stack.c rg_traverse.c rg_analysis.c rg_fixture.c rg_svg.c rg_alloc.c main.c
  tests/    rg_test.h test_main.c test_graph.c test_traverse.c test_analysis.c test_fixture_ref.c
  docs/     DESIGN.md FIXTURES.md TEST_PLAN.md TRACEABILITY.md reference/ evidence/
  out/      (gitignored; generated SVG/demo output)
```
`.gitignore`: build outputs, objects, executables, `out/`.

## 3. Data model (decisions already made, implement as stated)
- `RgGraph`: `int n_vertices`; vertex labels (`char label[32]`) and integer coordinates (`int x, y`, used only by the SVG export); canonical edge array `RgEdge {int u, v; bool active;}` with `n_edges`; adjacency MATRIX `int *matrix` (n*n, stores EDGE ID, or -1 for "no edge"); adjacency LIST using SINGLY LINKED LISTS: `RgAdjNode {int neighbor; int edge_id; struct RgAdjNode *next;}` per vertex.
- Active state lives ONLY in `edges[id].active`. Matrix and list both store edge IDs and consult `edges[id].active`, so disable/enable is O(1) and the two representations cannot drift apart.
- Adjacency lists must be in ASCENDING neighbor ID order (use sorted insertion, NOT head insertion). Matrix scans columns 0..n-1 ascending. This makes matrix and list traversals produce identical orders.
- Validation (return an `RgStatus` enum, never abort): reject n_vertices < 1; reject unknown endpoints; reject self-loops; reject duplicate undirected edges; reject invalid source/destination. A rejected operation must leave the graph unchanged. Vertex identity is the integer ID; labels are display only.
- Memory: every allocation goes through `rg_alloc.c` wrappers (`rg_malloc/rg_calloc/rg_free`) that count live allocations and support a TEST-ONLY "fail the Nth allocation" hook. `rg_graph_free` frees everything; document ownership in the header.
- `disable_edge` / `enable_edge` by edge ID and by endpoint pair. Disabling an already-disabled edge (or enabling an active one) is a harmless no-op that returns `RG_NOOP` (distinct from `RG_OK`). `rg_graph_reset` re-activates all edges.

## 4. Algorithms (implement explicitly, four separate traversal functions, no function-pointer abstraction, so they can be explained line by line in a viva)
- `bfs_matrix`, `bfs_list`: FIFO queue implemented in `rg_queue.c` (array-based; capacity n is enough since each vertex is enqueued at most once). Mark visited ON ENQUEUE. Record `pred[]` and `dist[]` (hops). Output: visit order, visited flags, pred, dist, `RgMetrics`.
- `dfs_matrix`, `dfs_list`: ITERATIVE DFS with an explicit stack from `rg_stack.c`. Rules: pop vertex; if already visited, count a stale pop and continue; otherwise mark visited ON POP, append to order, then push every unvisited neighbor in DESCENDING ID order (so the smallest ID is popped first). Stack capacity: 2*n_edges + 1. Also implement a RECURSIVE reference `dfs_list_recursive` used only by tests to cross-check that the iterative order equals recursive preorder.
- Route: BFS predecessor array reconstruction (walk back from destination, reverse). Unreachable destination returns `RG_NO_ROUTE` and an empty result, never a fabricated route. Source == destination returns the single-vertex path with 0 hops.
- Reachability: DFS-based `rg_reachable(src, dst)`; also derivable from BFS dist.
- Components: scan vertices in ascending ID order, start BFS from each unvisited vertex, assign a component ID per vertex, return the count. Provide both matrix and list versions.
- Impact summary after any mutation: reachable-vertex count from the source, component count, route hops (or "no route"), each shown as BEFORE -> AFTER.
- Single-failure impact table (`rg_failure_table`): for every edge, temporarily disable it, run BFS (list) from the source, record: destination reachable?, hop count (or none), reachable-vertex count, component count; re-enable it. After the loop assert the graph state is byte-identical to before. Label it honestly everywhere as "brute-force repeated BFS, O(E·(V+E)) on adjacency lists", NOT a bridge-finding algorithm.

## 5. Metrics (exact definitions, put them in `rg_traverse.h` and `docs/DESIGN.md`)
`RgMetrics`: `vertices_processed` (vertices dequeued by BFS / popped-and-not-stale by DFS), `neighbor_slots_examined`, `edges_skipped_inactive` (slot holds an edge ID whose edge is disabled), `queue_enqueues`, `queue_dequeues`, `stack_pushes`, `stack_pops`, `stale_pops`.
`neighbor_slots_examined`: matrix = every column j in 0..n-1 inspected for each processed vertex (includes j == u); list = every stored node inspected, including nodes of disabled edges. Do NOT count time. Optional `elapsed_ms` may exist but must be labelled informational and never asserted.

## 6. Fixtures (hand-verifiable; author's reference values below)
Fixture F1 "campus", 12 vertices, 13 edges, edge ID = order listed:
Vertices: 0 Main Gate, 1 Admin Block, 2 Library, 3 Central Plaza, 4 Block A, 5 Block B, 6 Hostel Junction, 7 Sports Complex, 8 Auditorium, 9 Health Centre, 10 Medical Annex, 11 Parking. (Pick integer coordinates for a readable SVG layout and record them.)
Edges: e0 (0,1) e1 (0,2) e2 (0,11) e3 (1,3) e4 (2,3) e5 (3,4) e6 (3,5) e7 (4,6) e8 (5,6) e9 (6,7) e10 (7,8) e11 (8,9) e12 (9,10).
Default demo: source 0, destination 10.

Reference values derived by the prompt author (NOT by the code). You must INDEPENDENTLY re-derive each one by hand-tracing the rules above and writing the trace into `docs/FIXTURES.md`. If your derivation disagrees with any value here, do not adjust either side silently: report the disagreement prominently in your final report and use your traced value only after showing the trace.
- BFS order from 0: 0,1,2,11,3,4,5,6,7,8,9,10. List metrics: processed 12, slots 26, enqueues 12, dequeues 12. Matrix: slots 144 (12 x 12).
- Route 0 -> 10: 0,1,3,4,6,7,8,9,10 (8 hops). (Equal-length alternatives exist; the tie policy is ascending-ID BFS discovery.)
- DFS order from 0: 0,1,3,2,4,6,5,7,8,9,10,11. List metrics: pushes 14, pops 14, stale_pops 2, processed 12, slots 26.
- Components on baseline: 1.
- Closures (source 0, destination 10):
  - disable e9 (6-7): no route; reachable from 0 = 8; components 2.
  - disable e10 (7-8): no route; reachable 9; components 2.
  - disable e11 (8-9): no route; reachable 10; components 2.
  - disable e12 (9-10): no route; reachable 11; components 2.
  - disable e2 (0-11): route unchanged (8 hops); reachable 11; components 2 (Parking isolated).
  - disable e5 (3-4): still reachable, 8 hops, route becomes 0,1,3,5,6,7,8,9,10; components 1.
  - disable any ONE of e0,e1,e3,e4,e6,e7,e8: still reachable, 8 hops, components 1.
- Failure table therefore shows exactly 4 edges (e9..e12) that disconnect the pair, and 5 edges (e2, e9..e12) that increase the component count.

Additional small edge-case fixtures (define in `rg_fixture.c`, trace by hand in FIXTURES.md):
- F2: n=7, edges (0,1),(1,2),(2,0),(3,4); vertices 5 and 6 isolated. Components = 4 ({0,1,2},{3,4},{5},{6}).
- F3: n=1, no edges (route 0->0 = [0], 0 hops, components 1).
- F4: chain n=4: 0-1-2-3.
- F5: 4-cycle 0-1-2-3-0: route 0->2 has two equal-length routes; ascending BFS gives 0,1,2.
- Also test: n=0 rejected; invalid IDs; duplicate edge `(1,0)` after `(0,1)`; self-loop.
Expected values in tests must be HARD-CODED from FIXTURES.md, never computed by the function under test.

## 7. Application
`main` is a menu-driven CLI (read input with `fgets` + `strtol`, no unchecked `scanf`, no buffer overflows): show graph; set source/destination; BFS traversal; DFS traversal; BFS route; reachability; components; disable edge; enable edge; reset; switch representation (matrix/list); failure table; export SVG; show metrics with the matrix-vs-list complexity note; quit. After every mutation, automatically recompute and print the BEFORE -> AFTER impact summary so results are never stale.
`./routeguard --demo` runs a fully scripted, deterministic scenario: baseline route 0->10, disable e9 (no route, show impact), re-enable, disable e5 (route changes, still 8 hops), reset, run BFS/DFS with both representations printing metrics side by side, print the failure table, write SVGs to `out/`. Output must be byte-identical across runs (assert this in a test or a `make demo-check` target by running twice and diffing).
SVG export (`rg_svg.c`, pure `fprintf`, no libraries): fixed coordinates from the fixture; active edges solid, disabled edges DASHED and labelled "closed" (never colour alone), route edges thick with hop numbers, unreached vertices drawn with a dashed outline and the text "unreached", title text "SYNTHETIC NETWORK — educational demo". Output must be well-formed XML (check with `xmllint --noout` if available, otherwise say it is unchecked). Rendering in a browser is NOT something you can verify; say so.

## 8. Tests and documentation
Tests: a tiny self-contained harness in `tests/rg_test.h` (assert macros, pass/fail counters, non-zero exit on any failure, prints totals). `make test` builds and runs everything. Each test is named with the SRS test ID it covers (RG-GR-01, RG-GR-02, RG-BFS-01..04, RG-DFS-01..02, RG-COMP-01, RG-FAIL-01..03, RG-REP-01..02, RG-MET-01, RG-ERR-01, RG-MEM-01). Required content:
- Both representations agree on reachability, distances, component count, and (given the identical neighbor order) traversal order and route.
- Iterative DFS order equals recursive reference order on every fixture.
- Exact metric values for F1 as listed above.
- Toggle sequences keep matrix/list consistent; reset restores baseline; the failure table leaves state unchanged.
- RG-MEM-01: repeated init/reset/free returns live-allocation count to 0; allocation-failure injection for graph creation and traversal setup leaves no leak and returns an error status. If you cannot implement part of this, mark it UNTESTED with the reason.
Docs (concise, no filler):
- `README.md`: what it is, scope + the Lecture 37 boundary, synthetic disclaimer, prerequisites actually found, exact build/test/run/demo commands you ran, project status table, name/regno placeholders `<STUDENT NAME>` `<REG NO>`.
- `AGENTS.md`: the hard rules from Section 0 for future agent sessions.
- `docs/DESIGN.md`: resolved decisions, data model, invariants (the 12 from SRS Section 4.3, each mapped to the test that checks it or marked UNCHECKED), algorithm descriptions with pseudocode, complexity table matched to what you ACTUALLY implemented (state any deviation from the SRS table), metric definitions, "Conflicts", "Decisions".
- `docs/FIXTURES.md`: every fixture with edge list, hand-traced BFS/DFS (step tables), expected counters, closure results.
- `docs/TEST_PLAN.md`: test ID -> function -> what it proves -> fixture.
- `docs/TRACEABILITY.md`: the SRS requirement-group table (FR-001..FR-040 grouped) with code location, test ID, status in {Not started, In progress, Implemented, Tested, Deferred}. Mark "Tested" ONLY if you ran the test this session. Mark all web UI / WASM items Deferred.

## 9. Commit plan (local only; one commit per line, commit only when that stage builds)
1. `Add project skeleton, build files, AGENTS.md, gitignore`
2. `Add graph model, validation, allocation wrappers`
3. `Add queue, stack, BFS/DFS (matrix and list), route reconstruction`
4. `Add components, impact summary, failure table, fixtures`
5. `Add CLI, demo mode, SVG export`
6. `Add test suite and test harness`
7. `Add documentation and traceability table`

## 10. Final report (your last message)
List: files created; toolchain actually found; exact commands run with their REAL output (build, `make strict`, `make test` totals, demo determinism diff); every disagreement with the author's reference values; every decision you made; every item marked UNTESTED/UNCHECKED and why; known risks. Then STOP and wait. Do not start verification hardening, reports, or slides.
````

---

## PROMPT B — Verify, harden, and collect evidence (run after you hand-checked the fixture)

````
# RouteGuard — Verification, Hardening and Evidence Prompt (single pass)

Context: the C11 RouteGuard engine, tests, CLI, SVG export and docs were built in the previous session (see `AGENTS.md`, `docs/DESIGN.md`, `docs/FIXTURES.md`). Your job now is to try to BREAK it and to record real evidence. Do not add features. Do not touch anything outside the RouteGuard directory. Local commits only, never push. Never claim a result without command output from this session.

## 1. Baseline gates (record raw output into `docs/evidence/`)
- `build.txt`: clean build with the documented command, plus `make strict` (-Werror). Fix real warnings in source; do not suppress them.
- `test.txt`: full test run, exact pass/fail totals.
- `env.txt`: refresh tool versions and OS.
- Sanitizers: run the whole test suite with `-fsanitize=address,undefined` if supported. If not supported on this machine (typical for MinGW on Windows), check whether WSL or valgrind is available and use it; otherwise write `sanitizer.txt` stating "UNAVAILABLE: <reason>" and mark memory-safety claims UNTESTED in the docs. Do not install anything without telling me first.

## 2. Mutation checks (prove the tests can fail)
Work in a TEMP COPY of the repo, never in the real tree. For each mutant below, apply it, run `make test`, record whether the suite FAILED (mutant killed) or PASSED (mutant survived), then revert:
1. BFS marks visited on dequeue instead of enqueue.
2. Adjacency list built with head insertion (reverses neighbor order).
3. Route reconstruction omits the source vertex.
4. `disable_edge` updates only the matrix view / only the list view (or stops honoring `active` in one traversal).
5. DFS marks visited on push instead of on pop.
6. Component counting skips isolated vertices.
7. `rg_graph_reset` fails to re-activate edges.
8. Duplicate-edge check removed.
9. Self-loop check removed.
10. Matrix scan loop runs to n-1 instead of n (off-by-one).
Write `mutation.txt` with a table: mutant, test(s) that killed it, or SURVIVED. For every survivor, add a focused test in the real repo that kills it, then re-run that mutant in the temp copy to confirm it is now killed. Report final kill rate honestly.

## 2b. Differential testing against an independent oracle (TEST CODE ONLY)
Add `tests/test_random.c`: a fixed-seed linear congruential generator (print the seed), 300 random graphs with n in 1..12, random edges (respecting the duplicate/self-loop rules), random disabled subsets. For each graph and each source, compare matrix BFS, list BFS, and an independent oracle that computes hop distances by repeated relaxation until no change (not queue-based) and component labels by label propagation. Compare iterative DFS order to the recursive reference. The oracle lives only in test code and is not part of the product. Record graph count, comparisons made, and any mismatch.

## 3. Reproducibility
- Fresh clone: `git clone` the local repo into a temp directory, run ONLY the commands written in the README, record output in `fresh_clone.txt`. If any README command fails or is missing a prerequisite, fix the README, not the evidence.
- Demo determinism: run `--demo` twice, diff, record in `demo_determinism.txt`; save one run's output as `demo_output.txt` and copy the generated SVGs to `docs/evidence/figures/`. Validate SVG well-formedness with `xmllint --noout` if available; state clearly that browser rendering is UNVERIFIED by you.
- Re-check the author's reference values from `docs/FIXTURES.md` against actual program output (BFS/DFS order, metrics 26/144, 14/14/2, route and closure table). List any mismatch.

## 4. Documentation truth pass
- Update `docs/TRACEABILITY.md`: set Tested ONLY where this session's output proves it; add the evidence file name per row. Keep web UI / WASM as Deferred.
- Update `README.md` status table and commands to match evidence exactly (test totals copied from `test.txt`).
- In `docs/DESIGN.md`, confirm each of the 12 invariants maps to a test or is marked UNCHECKED; reconcile the complexity table with the code (count actual loops in matrix vs list traversals) and note any deviation.
- Scan all docs for forbidden claims: any mention implying Dijkstra, heaps, bridges/articulation points, weighted routing, WASM, or React is implemented. Remove or label as non-goal.

## 5. Viva notes
Create `docs/VIVA_NOTES.md`: 25 likely examiner questions with concise answers grounded in THIS code, each citing file and function. Must cover: why BFS gives minimum hops; why DFS does not; mark-on-enqueue vs mark-on-pop and what breaks otherwise; why sorted insertion into adjacency lists; why edge IDs in the matrix; O(V²) vs O(V+E) with the 144 vs 26 numbers; stack capacity bound 2E+1; what stale pops are; how component counting works; what the failure table does and why it is NOT a bridge algorithm; what is deliberately out of scope (Dijkstra, heaps, low-link) and why; every known limitation and every UNTESTED item.

## 6. Evidence summary
Create `docs/evidence/SUMMARY.md` containing: tool versions, exact commands, build/strict results, test totals, sanitizer status, mutation kill table, differential test counts, fresh-clone result, determinism result, the F1 reference-value comparison table, list of figures, and an explicit "NOT VERIFIED" section. This file will be used to write the report and slides, so every number in it must be copied from a command output.

## 7. Commits and final report
Local commits: `Add mutation-derived tests and differential oracle`, `Update docs and traceability from verified evidence`, `Add viva notes and evidence summary`. Final message: what passed, what failed, what survived, what is UNTESTED, files changed. Then STOP.
````

---

## Suggested schedule (today is Sat 17 Oct)
| Day | Work |
|---|---|
| Sat–Sun | Prompt A, review the diff, hand-trace the fixture |
| Mon | Fix whatever you disagree with, run Prompt B |
| Tue | Bring evidence to chat; report PDF + slides built from it |
| Wed–Thu | Rehearse the demo and `VIVA_NOTES.md`; fresh-clone test again after any change |
| Fri | Buffer; submit early, keep the confirmation email |
