# COMPLETE DEMO GUIDE & PRESENTATION SCRIPT
## Deadlock Simulator — DAA Project
### Everything you need for tomorrow's presentation

---

# PART 1 — SAMPLE INPUT SESSIONS (Exact Keys to Type)

Each session below is a self-contained demonstration. Run them one at a time.

---

## SESSION 1 — Classic 3-Process Deadlock (Best for Viva Opening)

**What it proves:** DFS, Floyd–Warshall, and Topo Sort all agree there is a deadlock. Dijkstra picks the cheapest process to terminate.

```
Start the program:
  ./deadlock_sim

You will see the splash screen and menu.
```

**Step-by-step keystrokes (type each number and press Enter):**

```
16   ← Load predefined test case
1    ← Choose Test Case 1 (3-process deadlock: P0→P1→P2→P0)
```

The program loads 3 processes and 3 resources:
```
Alpha (P0) holds CPU,  requests Disk
Beta  (P1) holds Disk, requests Net
Gamma (P2) holds Net,  requests CPU   ← creates the circle
```

Now run each algorithm one by one so the examiner sees each output separately:

```
5    ← Show Resource Allocation Graph (see allocation + request edges)
6    ← Build Wait-For Graph (see "P0 waits for P1 via Disk" etc.)
7    ← DFS Detection  → OUTPUT: Cycle ▶ P0 → P1 → P2 → P0  ⚠ DEADLOCK
8    ← Floyd–Warshall → OUTPUT: reach[0][0]=reach[1][1]=reach[2][2]=1
9    ← Topo Sort      → OUTPUT: 0/3 vertices processed, NOT POSSIBLE
10   ← BFS from P0    (type 0 when asked for process id)
       → Level 0: P0, Level 1: P1, Level 2: P2 (chains show)
11   ← Dijkstra Recovery → Terminate P1 (Beta, cheapest cost=2)
12   ← Final Safe State → SAFE ✔, R1(Disk) freed
13   ← Performance comparison table
14   ← Export .dot file
```

---

## SESSION 2 — Partial Deadlock (5 Processes, Only 2 Are Stuck)

**What it proves:** DFS is smart — it finds ONLY the deadlocked processes, not all processes.

```
./deadlock_sim

16   ← Load test case
2    ← Test Case 2 (5 processes: P0↔P1 deadlocked, P2/P3/P4 safe)
```

What's set up:
```
P0 holds R1, requests R2 (held by P1) → P0 waits for P1
P1 holds R2, requests R1 (held by P0) → P1 waits for P0  ← 2-node cycle
P2 holds 1 instance of R3, requests another → NOT deadlocked (R3 has 2 instances)
```

Keystrokes:
```
6    ← Build WFG  → Only 2 edges: P0→P1 and P1→P0
7    ← DFS  → Cycle: P0 → P1 → P0  (only P0 and P1 flagged)
8    ← Floyd–Warshall → Only P0 and P1 have reach[i][i]=1. P2,P3,P4 = 0
9    ← Topo Sort → 2/5 vertices stuck (P0, P1). P2,P3,P4 processed fine.
11   ← Recovery → Terminate P0 (cost=3), P1 is freed → SAFE
12   ← Safe State → P1, P2, P3, P4 all survive
```

**Key talking point:** "Notice how Floyd–Warshall shows P2's diagonal as 0 — it is NOT in a cycle, even though it is waiting. This shows the precision of the algorithm."

---

## SESSION 3 — 4-Process Full Ring

**What it proves:** Larger cycle, all 4 processes stuck. Shows scalability.

```
./deadlock_sim

16   ← Load test case
3    ← Test Case 3 (4-process ring P0→P1→P2→P3→P0)
```

```
6    ← Build WFG → 4 wait-for edges
7    ← DFS → Cycle: P0 → P1 → P2 → P3 → P0
8    ← Floyd–Warshall → all 4 diagonals = 1
9    ← Topo Sort → 0/4 vertices, all stuck
13   ← Performance comparison (show timing table)
11   ← Recovery → Cheapest terminated, 3 processes survive
```

---

## SESSION 4 — Manual Input (Shows You Know the Internals)

**What it proves:** You understand how to build RAG from scratch. Impressive for viva.

```
./deadlock_sim

1    ← Add Process
Worker   ← name
5        ← recovery cost
3        ← priority

1    ← Add Process
Manager  ← name
8        ← recovery cost
5        ← priority

2    ← Add Resource
Printer  ← name
1        ← instances

2    ← Add Resource
Scanner  ← name
1        ← instances

3    ← Allocate resource
0        ← Process 0 (Worker)
0        ← Resource 0 (Printer)
1        ← 1 instance

3    ← Allocate resource
1        ← Process 1 (Manager)
1        ← Resource 1 (Scanner)
1        ← 1 instance

4    ← Request resource
0        ← Worker requests Scanner (held by Manager)
1        ← Resource 1
1        ← 1 instance

4    ← Request resource
1        ← Manager requests Printer (held by Worker)
0        ← Resource 0
1        ← 1 instance

5    ← Display RAG (show the edges you just created)
6    ← Build WFG → Worker→Manager, Manager→Worker
7    ← DFS → Cycle detected!
11   ← Recovery
12   ← Final state
```

**Talking point:** "I manually created a two-process deadlock: Worker holds Printer and wants Scanner. Manager holds Scanner and wants Printer. Neither can proceed."

---

## SESSION 5 — Random Graph (Show Randomness + Detection)

**What it proves:** The system works on any graph, not just hardcoded ones.

```
./deadlock_sim

17   ← Random graph generation
4    ← 4 processes
3    ← 3 resources

6    ← Build WFG
7    ← DFS (may or may not find deadlock — that's the point)
8    ← Floyd–Warshall (verify)
13   ← Performance report
```

**If no deadlock:** "This shows the system correctly identifies a SAFE state too. Detection means finding both states correctly."

---

## SESSION 6 — Full Auto Pipeline (Best for Live Demo Under Time Pressure)

**Use this if you only have 2 minutes:**

```
./deadlock_sim
16    ← Load test case
1     ← TC1
18    ← Full pipeline (runs everything automatically)
0     ← Exit
```

This runs: WFG → DFS → Floyd–Warshall → Topo Sort → BFS → Dijkstra → Safe State → Graphviz export — all in one shot.

---

---

# PART 2 — WHAT EACH OUTPUT MEANS (Read This Before Viva)

## Output 1: WFG Build

```
P0 waits for P1  (via R1: 'Disk')
P1 waits for P2  (via R2: 'Net')
P2 waits for P0  (via R0: 'CPU')
WFG built with 3 edge(s).
```

**Explain:** "The Wait-For Graph removes resource nodes from the RAG. Whenever process Pi requests a resource held by Pj, we draw a direct edge Pi→Pj. This simplifies deadlock analysis to a pure graph cycle problem."

---

## Output 2: DFS Detection

```
╔══════════════════════════════════════╗
║  ⚠  DEADLOCK DETECTED  (cycle #1)  ║
╚══════════════════════════════════════╝
  Cycle  ▶  P0 → P1 → P2 → P0
Deadlocked: Alpha, Beta, Gamma
Execution time: 2 μs
```

**Explain:** "DFS uses a 3-colour scheme: WHITE (unvisited), GRAY (currently on recursion stack), BLACK (done). When we find an edge going to a GRAY vertex, it means we found a back-edge — a path that goes back to an ancestor. That is a cycle. That is a deadlock. The parent array lets us trace exactly which processes are in the cycle."

**Why it is O(V+E):** "Each vertex changes colour only once: WHITE→GRAY→BLACK. Each edge is checked once. So total work is proportional to vertices plus edges."

---

## Output 3: Floyd–Warshall

```
Initial Matrix:       Final Reachability Matrix:
     P0 P1 P2              P0 P1 P2
P0 [  0  1  0  ]    P0 [  1  1  1  ]  ← P0 reaches itself!
P1 [  0  0  1  ]    P1 [  1  1  1  ]  ← P1 reaches itself!
P2 [  1  0  0  ]    P2 [  1  1  1  ]  ← P2 reaches itself!
```

**Explain:** "Floyd–Warshall computes the transitive closure — meaning, for every pair (i,j), can process i eventually reach process j through the chain of wait-for edges? The key check is the diagonal: if reach[i][i] = 1, process i can reach itself, meaning there is a cycle passing through it. All three diagonal entries are 1, confirming all three are deadlocked."

**Difference from DFS:** "DFS is faster — O(V+E) versus O(V³) — but Floyd–Warshall gives the complete picture of which processes can reach which other processes. It is used here as independent mathematical verification."

---

## Output 4: Topological Sort

```
In-degree table:
  P0 → 1
  P1 → 1
  P2 → 1

Kahn's BFS steps: (nothing runs — queue starts empty)

Topological order: 0/3 vertices
→ TOPOLOGICAL ORDERING NOT POSSIBLE
```

**Explain:** "Kahn's algorithm starts by finding all processes with in-degree 0 — processes that nobody is waiting for. It removes them and reduces in-degrees of their neighbours. If a process has no in-degree-0 node to start with, it means every process is being waited for by someone — that's a circular dependency. Here all three processes have in-degree 1, the queue starts empty, nothing gets processed, and 0 out of 3 vertices are ordered. Deadlock confirmed."

**Why topo sort fails for cycles:** "A topological ordering only exists for DAGs (Directed Acyclic Graphs). A cycle violates the acyclic property. So failure to complete the ordering is a mathematical proof of a cycle's existence."

---

## Output 5: BFS Traversal

```
Level 0: [P0]   P0 waits for: P1
Level 1: [P1]   P1 waits for: P2
Level 2: [P2]   P2 waits for: P0  ← circle closes
```

**Explain:** "BFS explores the wait-for dependencies level by level. Think of it as asking: if P0 is blocked, who does that block next? P1 at level 1, then P2 at level 2. When P2 points back to P0, we see the cycle close. This is useful for an OS administrator to visualise the blast radius — how far one blocked process propagates."

**Practical use:** "In a real OS, if P0 is a database server, BFS tells us exactly which other services get stuck if the database hangs."

---

## Output 6: Dijkstra Recovery

```
Recovery cost table:
  P0 Alpha  cost=4  priority=3
  P1 Beta   cost=2  priority=5
  P2 Gamma  cost=7  priority=2

► Victim: P1 (Beta)  [cost=2]

Strategy 1 – Termination: Terminate P1, release Disk
Strategy 2 – Preemption:  Reassign Disk to P0
Strategy 3 – Priority:    P1 (priority=5) sacrificed to save others

Recovery rounds: 1   |   Total cost: 2   |   Time: 41 μs

Surviving: P0 (Alpha), P2 (Gamma)
```

**Explain:** "Dijkstra's algorithm finds the shortest (cheapest) path through the recovery graph. Nodes are processes, edge weights are the cost to terminate the target process. Starting from the cheapest deadlocked process as source, Dijkstra computes minimum total cost to reach each other process. The minimum-cost victim is selected, terminated, its resources freed, and the WFG is rebuilt. We repeat until no deadlock remains."

**Why not just pick cheapest directly?** "A greedy pick of the single cheapest node ignores chain effects. Dijkstra considers the cumulative cost path — sometimes a slightly more expensive first termination enables cheaper subsequent ones. It gives the globally optimal sequence."

---

## Output 7: Final Safe State

```
✔  System is in a SAFE STATE.
✔  No deadlocked processes.

Resource availability:
  R0 (CPU):  [--------------------]  0/1 free
  R1 (Disk): [####################]  1/1 free  ← freed by recovery
  R2 (Net):  [--------------------]  0/1 free
```

**Explain:** "After terminating P1, the Disk resource is returned to the pool. P2 was waiting for CPU — which P0 still holds — but since P0 is no longer in a cycle (P1 is gone), P0 will eventually finish and release CPU. The system has reached a safe state: no more circular waiting."

---

## Output 8: Performance Table

```
Algorithm         Time        Space    Time(μs)
DFS Detection     O(V+E)      O(V)      2
Floyd–Warshall    O(V³)       O(V²)     4
Topological Sort  O(V+E)      O(V)      1
```

**Explain:** "For 3 processes the differences are tiny. But for 1000 processes: DFS and Topo Sort would take microseconds (linear), Floyd–Warshall would take seconds (cubic). This is why real OS kernels use DFS — O(V+E) is fast enough to run on every context switch."

---

---

# PART 3 — PRESENTATION FLOW (5-Minute Version)

## Minute 0–1: Introduction (Say This Exactly)

*"This project is a simulation of deadlock detection and recovery in operating systems using graph algorithms — which is a core DAA application. A deadlock happens when Process A waits for a resource held by B, B waits for something held by C, and C waits for A — a circle that can never break by itself.*

*We model this using two graphs: the Resource Allocation Graph, which shows who holds what and who wants what, and the Wait-For Graph, which is derived from it by removing resource nodes and directly connecting waiting processes. Once we have the WFG, deadlock detection becomes cycle detection — a classic graph theory problem."*

## Minute 1–2: Show the RAG and WFG (Type These)

```
./deadlock_sim
16 → 1 (load TC1)
5       (show RAG)
6       (build WFG, point at output)
```

*"Look at option 5 — the RAG shows R0 allocated to P0 via a green edge, and P0 requesting R1 via a dashed red edge. Option 6 converts this: since P0 requests R1 and R1 is held by P1, we draw P0→P1 in the WFG. Three such conversions give us this triangle: P0→P1→P2→P0."*

## Minute 2–3: DFS and Floyd–Warshall

```
7   (DFS)
8   (Floyd–Warshall)
```

*"Option 7 runs DFS. The cycle flag appears immediately — 2 microseconds. DFS marks each vertex as WHITE, GRAY when on the recursion stack, and BLACK when finished. Finding an edge back to a GRAY node is a back-edge — proof of a cycle — proof of deadlock.*

*Option 8 is independent mathematical verification using Floyd–Warshall. It computes the transitive closure — can process i reach itself? All three diagonal entries become 1. Both algorithms agree: deadlock confirmed."*

## Minute 3–4: Topo Sort and BFS

```
9   (topo sort)
10 → 0  (BFS from P0)
```

*"Topological sort is the third verification. Kahn's algorithm can only process in-degree-0 nodes. Here every process has in-degree 1 — the queue starts empty — zero vertices are ordered out of three. The algorithm fails, which is mathematical proof of a cycle.*

*BFS shows the human-readable chain: P0 waits for P1 at level 1, P1 waits for P2 at level 2, P2 waits back for P0 — the circle closes."*

## Minute 4–5: Recovery and Safe State

```
11  (Dijkstra)
12  (final state)
13  (performance)
```

*"Now Dijkstra recovery. Each process has a recovery cost — the cost of terminating it. Dijkstra finds the minimum-cost path through the deadlock, selects Beta (cost=2) as the victim, terminates it, frees the Disk resource. The WFG is rebuilt — no more cycle. System is SAFE. Total recovery cost: 2 units, 1 round, 41 microseconds.*

*The performance table compares all three algorithms: DFS and Topo Sort are both O(V+E) — linear and ideal for real-time kernels. Floyd–Warshall is O(V³) — gives complete reachability but doesn't scale past ~100 processes. In practice, Linux uses DFS-based detection."*

---

---

# PART 4 — EXPECTED VIVA QUESTIONS AND EXACT ANSWERS

**Q: "What is a deadlock?"**
A: "A deadlock is a state where a set of processes are each waiting for a resource held by another process in the set, so no process in the group can ever proceed."

**Q: "What are the four Coffman conditions?"**
A: "Mutual exclusion — resources can't be shared. Hold and Wait — a process holds one resource while requesting another. No preemption — resources can't be forcibly taken. Circular wait — processes form a cycle of dependencies. ALL four must hold simultaneously."

**Q: "Why does DFS detect deadlock?"**
A: "In the Wait-For Graph, edges mean 'this process is blocked waiting for that one.' A cycle in this graph means a set of processes are each waiting for the next — none can proceed. DFS finds cycles by detecting back-edges — edges that lead back to a node currently on the recursion stack."

**Q: "Why does Floyd–Warshall's diagonal matter?"**
A: "The transitive closure reach[i][j]=1 means 'process i can reach process j through wait-for edges.' If reach[i][i]=1, process i can reach itself — there is a directed path from i back to i — that is the definition of a cycle. A cycle equals deadlock."

**Q: "Why does Topological Sort fail for cyclic graphs?"**
A: "Kahn's algorithm removes nodes with in-degree 0 — nodes that no one is waiting for. In a cycle, every node has at least one incoming edge from its predecessor. So no node ever has in-degree 0, the queue stays empty, and the algorithm cannot process any node. Failure to order all nodes is proof of a cycle."

**Q: "What is the difference between RAG and WFG?"**
A: "RAG has both process and resource nodes. It has allocation edges from resources to processes, and request edges from processes to resources. WFG is derived by eliminating resource nodes and directly connecting processes: if Pi requests Rk and Pj holds Rk, we add edge Pi→Pj. WFG is simpler for cycle analysis."

**Q: "Why Dijkstra for recovery instead of just picking cheapest?"**
A: "A pure greedy approach picks the single cheapest process at each step. Dijkstra considers the cumulative cost path through the recovery graph — it finds the minimum total cost sequence. In some graphs, terminating a slightly more expensive process first enables much cheaper recovery of the remaining cycle."

**Q: "What is the time complexity of your DFS and why?"**
A: "O(V+E) where V is the number of processes and E is the number of wait-for edges. Every vertex is coloured exactly once — it changes from WHITE to GRAY to BLACK, three state changes total. Every edge is examined exactly once when scanning the adjacency row. So total operations are proportional to V + E."

**Q: "Can your system handle multiple deadlock cycles?"**
A: "Yes. The DFS outer loop restarts after detecting each cycle. Dijkstra recovery loops until no deadlocked process remains. TC2 in our project demonstrates partial deadlock — only 2 of 5 processes are stuck; the others are correctly identified as safe."

**Q: "What is the worst-case space complexity?"**
A: "The dominant structure is the Graph struct at about 7.6 KB. The WFG and reachability matrices are MAX_PROCESSES × MAX_PROCESSES × 4 bytes = 1.6 KB each. Floyd–Warshall uses O(V²) additional space for its local reach matrix. DFS and Topo Sort use O(V) space for their auxiliary arrays."

**Q: "How would you extend this to a real OS?"**
A: "Three ways: First, add Banker's Algorithm for proactive avoidance — grant allocations only when they lead to safe states. Second, generalise to multi-instance resources — our current WFG derivation handles multi-instance correctly via the allocation matrix. Third, add periodic detection: real kernels like Linux run a watchdog that triggers cycle detection when processes wait longer than a threshold."

---

---

# PART 5 — QUICK REFERENCE CARD (Keep This Open During Demo)

## Algorithm Summary

| Algo | What it does | Key output line to point at |
|------|-------------|----------------------------|
| DFS | Finds exact cycle path | `Cycle ▶ P0 → P1 → P2 → P0` |
| Floyd–Warshall | Verifies via reachability | `reach[0][0]=1 YES ⚠` |
| Topo Sort | Fails for cyclic graphs | `0/3 vertices ordered` |
| BFS | Shows chain level by level | `Level 0: P0 → Level 1: P1` |
| Dijkstra | Finds cheapest recovery | `Victim: P1 cost=2` |

## Menu Shortcut Sequences

| Demo Goal | Type These Options |
|-----------|--------------------|
| Full TC1 demo | 16, 1, 5, 6, 7, 8, 9, 10(→0), 11, 12, 13 |
| Quick auto run | 16, 1, 18 |
| Partial deadlock | 16, 2, 6, 7, 8, 12 |
| Manual deadlock | 1,1, 2,2, 3(0,0,1), 3(1,1,1), 4(0,1,1), 4(1,0,1), 6, 7 |
| Random + detect | 17, 4, 3, 6, 7, 8 |

## Phrases to Use

- "The Wait-For Graph converts a resource scheduling problem into a pure graph cycle problem."
- "DFS detects the cycle in O(V+E) — fast enough to run in every OS context switch."
- "Floyd–Warshall confirms with O(V³) — slower but gives the complete reachability picture."
- "Topological sort failure is a mathematical certificate that a cycle exists."
- "Dijkstra minimises recovery cost — we don't just kill processes blindly, we find the optimal sequence."
- "After termination, the WFG is rebuilt and re-checked — the loop continues until no deadlock remains."
