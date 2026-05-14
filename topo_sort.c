/*
 * topo_sort.c — Topological Sort Check (Kahn's BFS Algorithm)
 *
 * Logic:
 *   A directed graph can be topologically sorted if and only if it is
 *   a Directed Acyclic Graph (DAG).
 *   → If Kahn's algorithm processes ALL V vertices → no cycle → no deadlock.
 *   → If it processes fewer than V  → a cycle exists  → deadlock possible.
 *
 * Kahn's Algorithm:
 *   1. Compute in-degree for every vertex.
 *   2. Enqueue all vertices with in-degree 0.
 *   3. While queue non-empty:
 *        Dequeue u, add to ordering, decrement in-degree of neighbours.
 *        Enqueue any neighbour whose in-degree drops to 0.
 *   4. If |ordering| < V  → cycle detected.
 *
 * Complexity:
 *   Time : O(V + E)
 *   Space: O(V)
 */

#include "graph.h"

/* Simple integer queue using a circular array */
typedef struct { int data[MAX_PROCESSES]; int front, rear, size; } Queue;
static void q_init (Queue *q) { q->front = q->rear = q->size = 0; }
static int  q_empty(Queue *q) { return q->size == 0; }
static void q_push (Queue *q, int v)
{
    q->data[q->rear] = v;
    q->rear = (q->rear + 1) % MAX_PROCESSES;
    q->size++;
}
static int q_pop(Queue *q)
{
    int v = q->data[q->front];
    q->front = (q->front + 1) % MAX_PROCESSES;
    q->size--;
    return v;
}

/* ─────────────────────────────────────────────────────────────
   topo_sort_check: run Kahn's algorithm on the WFG
   ───────────────────────────────────────────────────────────── */
void topo_sort_check(const Graph *g)
{
    print_banner("TOPOLOGICAL SORT CHECK (KAHN'S ALGORITHM)");
    printf("  Algorithm : Kahn's BFS-based topological sort\n");
    printf("  Complexity : Time O(V+E)  |  Space O(V)\n");
    printf("  Principle  : Topo-sort succeeds ⟺ graph is a DAG ⟺ no deadlock\n\n");

    if (!g->wfg_built) {
        printf("  [!] WFG not built. Run option 6 first.\n\n");
        return;
    }

    int n = g->num_processes;
    if (n == 0) { printf("  (no processes)\n\n"); return; }

    long t_start = get_time_us();

    /* ── Step 1: Compute in-degrees ─────────────────────────── */
    int in_degree[MAX_PROCESSES] = {0};
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (g->wfg[i][j]) in_degree[j]++;

    printf("  In-degree table:\n");
    printf("  %-8s  %-10s\n", "Process", "In-Degree");
    printf("  %-8s  %-10s\n", "-------", "---------");
    for (int i = 0; i < n; i++)
        printf("  P%-7d  %d\n", i, in_degree[i]);
    printf("\n");

    /* ── Step 2: Enqueue in-degree-0 vertices ───────────────── */
    Queue q;
    q_init(&q);
    for (int i = 0; i < n; i++)
        if (in_degree[i] == 0) q_push(&q, i);

    /* ── Step 3: Process queue ──────────────────────────────── */
    int order[MAX_PROCESSES];
    int ord_len = 0;
    int step = 0;

    printf("  Kahn's BFS steps:\n");

    while (!q_empty(&q)) {
        int u = q_pop(&q);
        order[ord_len++] = u;
        printf("  Step %2d: dequeue P%d", ++step, u);

        int reduced = 0;
        for (int v = 0; v < n; v++) {
            if (g->wfg[u][v]) {
                in_degree[v]--;
                if (in_degree[v] == 0) {
                    q_push(&q, v);
                    if (!reduced) printf("  →  reduce in-deg of:");
                    printf(" P%d", v);
                    reduced = 1;
                }
            }
        }
        printf("\n");
    }

    long t_end = get_time_us();

    /* ── Step 4: Verdict ────────────────────────────────────── */
    printf("\n  Topological order obtained (%d/%d vertices): ", ord_len, n);
    for (int i = 0; i < ord_len; i++) {
        printf("P%d", order[i]);
        if (i < ord_len - 1) printf(" → ");
    }
    printf("\n\n");

    if (ord_len == n) {
        printf("  ✔  Topological ordering COMPLETE.\n");
        printf("  ✔  All %d processes ordered → Graph is a DAG.\n", n);
        printf("  ✔  No cycle → No deadlock detected by topo sort.\n");
    } else {
        printf("  ╔══════════════════════════════════════════════════╗\n");
        printf("  ║  ⚠  TOPOLOGICAL ORDERING NOT POSSIBLE           ║\n");
        printf("  ║     Graph contains a cycle.                      ║\n");
        printf("  ║     Potential deadlock exists!                   ║\n");
        printf("  ╚══════════════════════════════════════════════════╝\n");
        printf("\n  Unschedulable processes (still have in-degree > 0):\n");
        for (int i = 0; i < n; i++) {
            if (in_degree[i] > 0)
                printf("    • P%d (%s)  in-degree = %d\n",
                       i, g->processes[i].name, in_degree[i]);
        }
    }

    printf("\n  Execution time : %ld μs\n\n", t_end - t_start);
}
