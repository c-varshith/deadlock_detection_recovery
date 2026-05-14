# PRESENTATION SLIDES
## Algorithmic Detection and Resolution of Deadlocks in RAGs
### Design and Analysis of Algorithms — Term Project

---

## SLIDE 1 — Title

**Title:** Algorithmic Detection and Resolution of Deadlocks in Resource Allocation Graphs

**Subtitle:** A C-language Simulation using DFS · Floyd–Warshall · Topological Sort · BFS · Dijkstra

---

## SLIDE 2 — What is Deadlock?

**Definition:**
A deadlock is a state where a group of processes are **permanently blocked** because each is waiting for a resource held by another process in the group.

**Four Coffman Conditions (ALL must hold):**

| Condition         | Meaning                                   |
|-------------------|-------------------------------------------|
| Mutual Exclusion  | Resources cannot be shared                |
| Hold and Wait     | Process holds one resource, waits for more|
| No Preemption     | Resources cannot be forcibly taken away   |
| Circular Wait     | Processes form a circular waiting chain   |

**Diagram:**  P1 → P2 → P3 → P1  ← deadlock cycle

---

## SLIDE 3 — Project Goal

**Objective:** Build a complete simulation of deadlock detection and resolution using graph algorithms.

**Pipeline:**
```
Input Data → RAG → WFG → DFS → Floyd–Warshall → Topo Sort → BFS → Dijkstra Recovery → Safe State
```

**Why this matters:**
- Every modern OS faces this problem
- Wrong handling wastes CPU time and starves processes
- Optimal recovery saves maximum processes at minimum cost

---

## SLIDE 4 — System Architecture

```
┌─────────────────────────────────────────────┐
│              main.c  (18-option menu)        │
├────────────┬────────────┬────────────────────┤
│  graph.c   │   rag.c    │     wfg.c          │
│  Entities  │  RAG build │  WFG derivation    │
├────────────┴────────────┴────────────────────┤
│  dfs_deadlock.c  │  floyd_warshall.c         │
│  O(V+E)          │  O(V³)                    │
├──────────────────┴───────────────────────────┤
│  topo_sort.c  │  bfs.c  │ dijkstra_recovery.c│
│  O(V+E)       │  O(V+E) │  O(V²)             │
├───────────────┴─────────┴────────────────────┤
│  visualization.c   │   utils.c               │
│  ASCII + Graphviz  │   Timing, matrices       │
└────────────────────┴────────────────────────┘
```

---

## SLIDE 5 — Resource Allocation Graph

**RAG encodes two types of edges:**

```
Process → Resource:  REQUEST edge (process wants resource)
Resource → Process:  ALLOCATION edge (resource is held by process)
```

**Example (Test Case 1):**
```
(P0) - - → [R1]       P0 holds CPU, requests Disk
[R0] ────→ (P0)
(P1) - - → [R2]       P1 holds Disk, requests Net
[R1] ────→ (P1)
(P2) - - → [R0]       P2 holds Net, requests CPU ← CYCLE!
[R2] ────→ (P2)
```

**Deadlock detected:** P0 waits for P1 waits for P2 waits for P0

---

## SLIDE 6 — Algorithm 1: DFS Detection

**Complexity: O(V + E)**

**Three-colour DFS:**
- WHITE → unvisited
- GRAY  → on recursion stack (ancestor)
- BLACK → fully processed

**Key insight:** A back-edge to a GRAY vertex = cycle = deadlock

**Cycle path extraction:**
```
parent[] array traces from cycle_end → cycle_start
Printed in reverse: P0 → P1 → P2 → P0
```

**Output:**
```
╔══════════════════════════════════════╗
║  ⚠  DEADLOCK DETECTED  (cycle #1)  ║
╚══════════════════════════════════════╝
  Cycle  ▶  P0 → P1 → P2 → P0
```

---

## SLIDE 7 — Algorithm 2: Floyd–Warshall

**Complexity: O(V³)**

**Transitive closure recurrence:**
```
reach[i][j] = reach[i][j] OR (reach[i][k] AND reach[k][j])
```

**Deadlock criterion:** `reach[i][i] = 1` means Pi can reach itself → cycle

**Reachability matrix after closure (TC1):**
```
     P0  P1  P2
P0 [  1   1   1  ]   ← P0 in cycle
P1 [  1   1   1  ]   ← P1 in cycle
P2 [  1   1   1  ]   ← P2 in cycle
```
All diagonal entries = 1 → all three processes deadlocked.

**Advantage:** Identifies ALL mutually-waiting pairs in a single O(V³) pass.

---

## SLIDE 8 — Algorithm 3: Topological Sort

**Complexity: O(V + E)  |  Algorithm: Kahn's BFS**

**Logic:**
- A graph is cycle-free ↔ it can be topologically ordered
- Kahn's removes in-degree-0 vertices iteratively
- If it can't process all V vertices → cycle exists → deadlock

**Test Case 1:**
```
In-degrees: P0=1, P1=1, P2=1
Queue initially empty (no zero in-degree)
Processed: 0 / 3 vertices
→ TOPOLOGICAL ORDERING NOT POSSIBLE
→ DEADLOCK EXISTS
```

**Unschedulable processes:** P0 (Alpha), P1 (Beta), P2 (Gamma)

---

## SLIDE 9 — Algorithm 4: BFS Traversal

**Complexity: O(V + E)**

**Purpose:** Show the propagation of blocking — the "blast radius"

**Level-by-level output (from P0):**
```
Level 0: [P0]   P0 waits for: P1
Level 1: [P1]   P1 waits for: P2
Level 2: [P2]   P2 waits for: P0   ← cycle closes here
```

**Use case:**
An OS administrator can see: "P0 is blocked, causing P1 and P2 to also be stuck."

---

## SLIDE 10 — Algorithm 5: Dijkstra Recovery

**Complexity: O(V²) per round**

**Model:**
- Nodes = processes
- Edge weights = recovery cost of target process
- Find minimum-cost termination sequence

**Recovery table (TC1):**

| Process | Name  | Cost | Priority |
|---------|-------|------|----------|
| P0      | Alpha |  4   |    3     |
| P1      | Beta  |  2   |    5     |
| P2      | Gamma |  7   |    2     |

**Dijkstra selects P1 (cost=2) → terminate → deadlock resolved**

**Three strategies applied:**
1. **Termination:** Kill P1, free its Disk resource
2. **Preemption:** Reassign Disk to waiting P0
3. **Priority-based:** P1 (priority 5) sacrificed to save P0 (3) and P2 (2)

---

## SLIDE 11 — Comparison of Algorithms

| Algorithm         | Time     | Space  | Detects    | Best Use             |
|-------------------|----------|--------|------------|----------------------|
| DFS               | O(V+E)   | O(V)   | Exact cycle| Real-time OS kernel  |
| Floyd–Warshall    | O(V³)    | O(V²)  | All pairs  | Batch analysis       |
| Topological Sort  | O(V+E)   | O(V)   | Cycle exists| Scheduler design    |
| BFS               | O(V+E)   | O(V)   | Chain depth | Admin diagnosis     |
| Dijkstra          | O(V²)    | O(V)   | Recovery   | Resource management  |

**Winner for detection:** DFS (linear time, exact cycle path)
**Winner for verification:** Floyd–Warshall (complete reachability)
**Winner for recovery:** Dijkstra (minimum cost)

---

## SLIDE 12 — Test Results

**Test Case 1 — 3-process full deadlock:**
- All three detection algorithms agree: DEADLOCK
- Recovery: Terminate P1 (cost 2) → system safe
- Total cost: 2 units

**Test Case 2 — 5-process partial deadlock:**
- Only P0 ↔ P1 deadlocked; P2, P3, P4 safe
- DFS correctly identifies only P0 and P1
- Floyd–Warshall: only reach[0][0] and reach[1][1] = 1

**Test Case 3 — 4-process ring:**
- Full ring P0→P1→P2→P3→P0
- All four processes deadlocked
- Recovery terminates minimum-cost process; 3 survivors

---

## SLIDE 13 — Output Files

**Generated by the simulator:**

| File                  | Contents                              |
|-----------------------|---------------------------------------|
| `outputs/rag.dot`     | Graphviz RAG with colour-coded nodes  |
| `outputs/wfg.dot`     | Graphviz WFG with wait-for edges      |
| Terminal output       | Full algorithm traces with timing     |

**Rendering command:**
```bash
dot -Tpng outputs/rag.dot -o rag.png
```

**Legend:**
- Blue ellipse = process node (red = deadlocked)
- Green box   = resource node
- Green solid arrow = allocation  (R → P)
- Red dashed arrow  = request     (P → R)
- Purple bold arrow = wait-for    (P → P, WFG overlay)

---

## SLIDE 14 — Conclusion

**Achievements:**
- ✔ Fully functional menu-driven C simulator (14 source files)
- ✔ Five graph algorithms implemented from scratch
- ✔ Correct deadlock detection on all three test cases
- ✔ Optimal minimum-cost recovery using Dijkstra
- ✔ Graphviz export for professional visualisation
- ✔ Performance benchmark comparing all algorithms

**Key DAA takeaway:**
Graph algorithms (DFS, BFS, Floyd–Warshall, Topo Sort, Dijkstra) are not
abstract theory — they directly power operating system resource management.

---

## SLIDE 15 — References

1. Cormen, T.H. et al. *Introduction to Algorithms*, 4th ed. MIT Press, 2022.
2. Silberschatz, A. et al. *Operating System Concepts*, 10th ed. Wiley, 2021.
3. Dijkstra, E.W. "A note on two problems in connexion with graphs." 1959.
4. Karp, R.M. "A characterization of the minimum mean-cycle in a digraph." 1978.
5. Coffman, E.G. et al. "System deadlocks." *ACM Computing Surveys*, 1971.
