# RouteGuard — Agent Guidelines & Hard Boundaries

This document defines mandatory operational constraints and academic scope for all AI coding agents working on the RouteGuard repository.

## 1. Absolute Hard Rules
- **Language**: Pure ISO C11 standard library only (`<stdio.h>`, `<stdlib.h>`, `<stdbool.h>`, `<string.h>`, `<errno.h>`). No C++ code, no C++ compiler flags, no third-party libraries.
- **Academic Scope Boundary**: Strictly bounded to topics covered through CSC2101 Lecture 37 (Introductory Data Structures & Algorithms).
  - **Permitted**: Fixed arrays, singly linked lists, FIFO queues, LIFO stacks, recursion, adjacency matrix, adjacency list, Breadth-First Search (BFS), Depth-First Search (DFS), connected components via repeated traversal, time/space complexity analysis.
  - **Strictly FORBIDDEN**: Dijkstra, A*, heaps / priority queues, Minimum Spanning Trees (Kruskal/Prim), Disjoint Set / Union-Find, low-link / Tarjan / Kosaraju bridge or articulation algorithms, hash tables, balanced/binary trees. Do NOT introduce them even as bonuses or optimizations.
- **Graph Topology**: The graph is strictly **UNDIRECTED** and **UNWEIGHTED**. Route length equals the count of edges (hops), never physical distance or transit duration.
- **Data Integrity & Synthetic Disclaimer**: All data and vertex names are purely synthetic (educational campus topology). Never claim suitability for real-world navigation, municipal planning, or emergency dispatch.
- **Isolation**: Never modify, move, delete, or copy files from any repository named LogWatch.
- **Target Location**: `C:\Soham\Coding\GitHub\RouteGuard`. Never overwrite user work without checking `git status`.
- **Git Discipline**: Local commits only using the approved commit plan. Never push to any remote; never modify remote configuration.
- **Verification Honesty**: Never claim "passes", "verified", or document test numbers unless directly produced and pasted from command output in the current session. All unexecuted features or tests must be explicitly marked `UNTESTED` along with the factual reason.
- **Specification Authority**: `docs/reference/RouteGuard_SRS_v0_2.pdf` is authoritative. Any discrepancy between prompts and the SRS must be logged in `docs/DESIGN.md` under "Conflicts".
