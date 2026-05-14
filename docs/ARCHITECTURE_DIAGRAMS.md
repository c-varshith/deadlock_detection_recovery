# ARCHITECTURE & FLOWCHART DIAGRAMS

## 1. System Architecture

```
╔══════════════════════════════════════════════════════════════════╗
║                    DEADLOCK SIMULATOR                            ║
║              (C Language — Modular Architecture)                 ║
╠══════════════════════════════════════════════════════════════════╣
║                                                                  ║
║  ┌─────────────────────────────────────────────────────────┐    ║
║  │                    main.c                                │    ║
║  │            18-Option Menu-Driven Interface               │    ║
║  └───┬─────┬──────┬──────┬──────┬──────┬──────┬───────────┘    ║
║      │     │      │      │      │      │      │                  ║
║  ┌───▼─┐ ┌─▼──┐ ┌─▼──┐ ┌▼────┐ ┌▼────┐ ┌▼───┐ ┌▼──────────┐  ║
║  │graph│ │rag │ │wfg │ │ dfs │ │floyd│ │topo│ │  dijkstra  │  ║
║  │  .c │ │ .c │ │ .c │ │ dl  │ │ war │ │sort│ │ recovery.c │  ║
║  │     │ │    │ │    │ │ .c  │ │ .c  │ │ .c │ │            │  ║
║  └─────┘ └────┘ └────┘ └─────┘ └─────┘ └────┘ └────────────┘  ║
║                                                                  ║
║  ┌─────────────────────┐   ┌───────────────┐   ┌─────────────┐ ║
║  │   bfs.c             │   │visualization.c│   │   utils.c   │ ║
║  │ BFS traversal       │   │ASCII + Graphviz│  │Timing, fmt  │ ║
║  └─────────────────────┘   └───────────────┘   └─────────────┘ ║
║                                                                  ║
║  ┌──────────────────────────────────────────────────────────┐   ║
║  │                    graph.h                                │   ║
║  │  Process · Resource · Graph structs · All declarations    │   ║
║  └──────────────────────────────────────────────────────────┘   ║
╚══════════════════════════════════════════════════════════════════╝
```

---

## 2. Full System Flowchart

```
                        ┌──────────────┐
                        │     START    │
                        └──────┬───────┘
                               │
                        ┌──────▼───────┐
                        │  Load Input  │
                        │ ─────────── │
                        │ • Manual    │
                        │ • Random    │
                        │ • Test Case │
                        └──────┬───────┘
                               │
                        ┌──────▼───────────────┐
                        │   Build RAG           │
                        │ ─────────────────── │
                        │ allocation[P][R]      │
                        │ request[P][R]         │
                        └──────┬────────────────┘
                               │
                        ┌──────▼───────────────┐
                        │   Derive WFG          │
                        │ ─────────────────── │
                        │ For each Pi→Rk and    │
                        │ Pj holds Rk:          │
                        │ Add edge Pi→Pj in WFG │
                        └──────┬────────────────┘
                               │
               ┌───────────────┼───────────────┐
               │               │               │
        ┌──────▼─────┐  ┌──────▼──────┐ ┌─────▼──────┐
        │   DFS       │  │Floyd–Warsh. │ │  Topo Sort  │
        │ O(V+E)      │  │   O(V³)     │ │  O(V+E)     │
        │ Cycle detect│  │reach[i][i]  │ │ Kahn's algo │
        └──────┬─────┘  └──────┬──────┘ └─────┬──────┘
               │               │               │
               └───────────────┼───────────────┘
                               │
                               ▼
                      ┌─────────────────┐
                      │   Deadlock?     │
                      └────┬──────┬─────┘
                         Yes      No
                          │        │
               ┌──────────▼──┐  ┌──▼──────────────────┐
               │ BFS Explore │  │  System is SAFE      │
               │ (chains)    │  │  No action needed    │
               └──────┬──────┘  └─────────────────────┘
                      │
               ┌──────▼────────────────┐
               │  Dijkstra Recovery    │
               │ ─────────────────── │
               │ 1. Find min-cost node │
               │ 2. Terminate process  │
               │ 3. Free resources     │
               │ 4. Rebuild WFG        │
               │ 5. Repeat if needed   │
               └──────┬────────────────┘
                      │
               ┌──────▼────────────────┐
               │    Final Safe State   │
               │ ─────────────────── │
               │ Surviving processes   │
               │ Available resources   │
               │ Recovery cost summary │
               └──────┬────────────────┘
                      │
               ┌──────▼────────────┐
               │  Export Graphviz  │
               │  outputs/rag.dot  │
               │  outputs/wfg.dot  │
               └──────┬────────────┘
                      │
                 ┌─────▼────┐
                 │   END    │
                 └──────────┘
```

---

## 3. RAG → WFG Conversion Diagram

```
RESOURCE ALLOCATION GRAPH (RAG)         WAIT-FOR GRAPH (WFG)
                                         (resources removed)

  (P0) ──────────────── [R1]
       ◄── allocated ──
       ── requests ──►                    (P0) ──────────────► (P1)

  (P1) ──────────────── [R1]
       ◄── allocated ──
       ── requests ──► [R2]              (P1) ──────────────► (P2)

  (P2) ──────────────── [R2]
       ◄── allocated ──
       ── requests ──► [R0]              (P2) ──────────────► (P0)

  (P0) ◄── allocated ── [R0]

Conversion rule:
  Pi → Rk (request)   +   Rk → Pj (allocation)
  ════════════════════════════════════════════
                 Pi → Pj  (wait-for edge)
```

---

## 4. DFS State Diagram

```
Vertex states during DFS:

  WHITE ──────────────────────────────────────────► GRAY
  (unvisited)    dfs_visit(u) called          (on stack)
                                                    │
                                       ┌────────────┘
                                       │
                      outgoing edge u→v found
                                       │
                    ┌──────────────────┴──────────────┐
                    │                                  │
             colour[v]==GRAY                    colour[v]==WHITE
                    │                                  │
           ╔════════▼════════╗              recursively visit v
           ║  CYCLE DETECTED  ║                        │
           ║  (back-edge u→v) ║                        │
           ╚══════════════════╝                        │
                                                colour[u] = BLACK
                                                (fully processed)
```

---

## 5. Dijkstra Recovery Iteration Model

```
Iteration 1:
  Deadlocked: {P0(cost=4), P1(cost=2), P2(cost=7)}
  Run Dijkstra from P1 (cheapest source)
  Select victim = P1 (min-cost reachable)
  Terminate P1 → free R1(Disk)
  Rebuild WFG...

Iteration 2 (if needed):
  Check remaining processes
  No cycle → SAFE
  Stop.

Total cost = 2 (just P1)
```
