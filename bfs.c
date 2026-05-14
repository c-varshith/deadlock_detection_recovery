/*
 * bfs.c — BFS Traversal on the Wait-For Graph
 *
 * Purpose:
 *   Explore the dependency chain starting from a given process.
 *   BFS levels reveal HOW FAR the blocking propagates:
 *     Level 0: the starting process
 *     Level 1: processes it directly waits for
 *     Level 2: processes those wait for  ... and so on.
 *
 * This helps visualise the "blast radius" of a blocked process.
 *
 * Complexity:
 *   Time : O(V + E)
 *   Space: O(V)  (visited array + queue)
 */

#include "graph.h"

/* ─────────────────────────────────────────────────────────────
   bfs_traversal: BFS from a single starting process
   ───────────────────────────────────────────────────────────── */
void bfs_traversal(const Graph *g, int start_pid)
{
    int n = g->num_processes;
    if (start_pid < 0 || start_pid >= n) {
        printf("  [!] Invalid process id: %d\n\n", start_pid);
        return;
    }
    if (!g->wfg_built) {
        printf("  [!] WFG not built. Run option 6 first.\n\n");
        return;
    }

    int visited[MAX_PROCESSES] = {0};
    int queue  [MAX_PROCESSES];
    int level  [MAX_PROCESSES]; /* BFS level of each vertex */
    int front = 0, rear = 0;

    queue[rear]       = start_pid;
    level[rear]       = 0;
    rear++;
    visited[start_pid] = 1;

    printf("  BFS from P%d (%s):\n\n",
           start_pid, g->processes[start_pid].name);

    int current_level = -1;

    while (front < rear) {
        int u   = queue[front];
        int lev = level[front];
        front++;

        if (lev != current_level) {
            current_level = lev;
            printf("  Level %d: ", lev);
        }
        printf("[P%d] ", u);

        /* Print waiting description */
        int any_wait = 0;
        for (int v = 0; v < n; v++) {
            if (g->wfg[u][v]) {
                if (!any_wait) {
                    printf("\n    P%d waits for:", u);
                    any_wait = 1;
                }
                printf("  P%d", v);
                if (!visited[v]) {
                    visited[v]    = 1;
                    queue[rear]   = v;
                    level[rear]   = lev + 1;
                    rear++;
                }
            }
        }
        if (!any_wait) printf("\n    P%d → (not waiting for any process)\n", u);
        else printf("\n");
    }
    printf("\n");

    /* Summary of reachable processes */
    printf("  Processes reachable from P%d:\n", start_pid);
    for (int i = 0; i < n; i++) {
        if (visited[i] && i != start_pid)
            printf("    P%d (%s)\n", i, g->processes[i].name);
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
   bfs_all: run BFS from every process as starting node
   ───────────────────────────────────────────────────────────── */
void bfs_all(const Graph *g)
{
    print_banner("BFS DEPENDENCY TRAVERSAL (ALL PROCESSES)");
    printf("  Algorithm : Breadth-First Search on WFG\n");
    printf("  Complexity : O(V + E) per source\n\n");

    if (!g->wfg_built) {
        printf("  [!] WFG not built. Run option 6 first.\n\n");
        return;
    }

    long t_start = get_time_us();

    for (int i = 0; i < g->num_processes; i++) {
        print_separator();
        bfs_traversal(g, i);
    }

    long t_end = get_time_us();
    printf("  Total BFS execution time : %ld μs\n\n", t_end - t_start);
}
