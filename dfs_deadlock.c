/*
 * dfs_deadlock.c — DFS-based Deadlock Detection on the Wait-For Graph
 *
 * Algorithm:
 *   Standard DFS with a "recursion stack" colour scheme:
 *     WHITE (0) — not yet visited
 *     GRAY  (1) — currently on DFS stack (ancestor)
 *     BLACK (2) — fully processed
 *
 *   A back-edge (u → v where v is GRAY) reveals a cycle.
 *
 * Complexity:
 *   Time:  O(V + E)   where V = processes, E = WFG edges
 *   Space: O(V)       for colour array, parent array, and recursion stack
 *
 * Why cycle ⟹ deadlock:
 *   Every edge Pi → Pj means "Pi is blocked waiting for a resource
 *   held by Pj."  A cycle P1→P2→…→Pk→P1 means every process in the
 *   cycle is waiting for the next one — none can proceed.
 */

#include "graph.h"

/* Internal colour constants */
#define WHITE 0   /* unvisited          */
#define GRAY  1   /* on recursion stack */
#define BLACK 2   /* finished           */

/* Module-level state (reset each call) */
static int colour[MAX_PROCESSES];
static int parent[MAX_PROCESSES];
static int cycle_start;      /* first vertex where back-edge was detected */
static int cycle_end;        /* vertex that detected the back-edge         */
static int n_proc;           /* local copy of num_processes                */
static const Graph *G;       /* pointer to graph being analysed            */

/* ─────────────────────────────────────────────────────────────
   print_cycle: trace back through parent[] to reconstruct path
   ───────────────────────────────────────────────────────────── */
static void print_cycle(void)
{
    /* Build cycle by following parent pointers from cycle_end
       back to cycle_start, then print in reverse.             */
    int path[MAX_PROCESSES];
    int len = 0;
    int v = cycle_end;
    while (v != cycle_start && len < MAX_PROCESSES) {
        path[len++] = v;
        v = parent[v];
    }
    path[len++] = cycle_start;

    /* Reverse for forward order */
    printf("    Cycle  ▶  ");
    for (int i = len - 1; i >= 0; i--) {
        printf("P%d", path[i]);
        if (i > 0) printf(" → ");
    }
    printf(" → P%d\n", cycle_start);   /* close the cycle */
}

/* ─────────────────────────────────────────────────────────────
   dfs_visit: recursive DFS visit from vertex u
   Returns 1 if a cycle was found, 0 otherwise
   ───────────────────────────────────────────────────────────── */
static int dfs_visit(int u)
{
    colour[u] = GRAY;

    for (int v = 0; v < n_proc; v++) {
        if (!G->wfg[u][v]) continue;

        if (colour[v] == GRAY) {
            /* Back-edge found → cycle */
            cycle_start = v;
            cycle_end   = u;
            return 1;
        }
        if (colour[v] == WHITE) {
            parent[v] = u;
            if (dfs_visit(v)) return 1;
        }
    }
    colour[u] = BLACK;
    return 0;
}

/* ─────────────────────────────────────────────────────────────
   dfs_detect_deadlock: public entry point
   Returns 1 if deadlock found, 0 if safe
   ───────────────────────────────────────────────────────────── */
int dfs_detect_deadlock(Graph *g)
{
    print_banner("DFS DEADLOCK DETECTION");
    printf("  Algorithm : Depth-First Search with recursion-stack colouring\n");
    printf("  Complexity : Time O(V+E)  |  Space O(V)\n");
    printf("  V = %d processes,  E = WFG edges\n\n", g->num_processes);

    if (!g->wfg_built) {
        printf("  [!] WFG not built — building now...\n");
        wfg_build(g);
    }

    long t_start = get_time_us();

    G      = g;
    n_proc = g->num_processes;
    for (int i = 0; i < n_proc; i++) {
        colour[i] = WHITE;
        parent[i] = -1;
    }
    cycle_start = -1;
    cycle_end   = -1;

    int deadlock = 0;
    int cycle_count = 0;

    for (int i = 0; i < n_proc; i++) {
        if (colour[i] == WHITE) {
            if (dfs_visit(i)) {
                deadlock = 1;
                cycle_count++;
                printf("  ╔══════════════════════════════════════╗\n");
                printf("  ║  ⚠  DEADLOCK DETECTED  (cycle #%d)    ║\n", cycle_count);   
                printf("  ╚══════════════════════════════════════╝\n");
                print_cycle();

                /* Mark deadlocked processes */
                int v = cycle_end;
                while (v != cycle_start) {
                    g->processes[v].is_deadlocked = 1;
                    v = parent[v];
                }
                g->processes[cycle_start].is_deadlocked = 1;

                /* Reset to detect more cycles */
                for (int j = 0; j < n_proc; j++) {
                    colour[j] = WHITE;
                    parent[j] = -1;
                }
                cycle_start = -1;
                cycle_end   = -1;
                i = -1;     /* restart outer loop */
                break;      /* one cycle per outer iteration */
            }
        }
    }

    long t_end = get_time_us();

    if (!deadlock) {
        printf("  ✔  No deadlock detected by DFS.\n");
        printf("  All processes can eventually make progress.\n");
    } else {
        printf("\n  Deadlocked processes:\n");
        for (int i = 0; i < g->num_processes; i++) {
            if (g->processes[i].is_deadlocked)
                printf("    • P%d (%s)\n", i, g->processes[i].name);
        }
        g->deadlock_found = 1;
    }

    printf("\n  Execution time : %ld μs\n\n", t_end - t_start);
    return deadlock;
}
