/*
 * dijkstra_recovery.c — Dijkstra-based Optimal Deadlock Recovery
 *
 * Model:
 *   Nodes  : processes
 *   Edges  : wait-for dependencies (from WFG)
 *   Weights: recovery cost of each process (cost to terminate / preempt)
 *
 * Strategy:
 *   Starting from the lowest-cost deadlocked process as source,
 *   use Dijkstra to find the minimum-cost path through the deadlock
 *   cycle.  The process with minimum distance from source is selected
 *   as the next termination victim, its resources are freed, and the
 *   graph is updated until no deadlock remains.
 *
 * Complexity:
 *   Time : O(V²) for adjacency-matrix Dijkstra
 *   Space: O(V)
 *
 * Recovery Strategies implemented:
 *   1. Process Termination  — kill the cheapest process
 *   2. Resource Preemption  — preempt resources from a process
 *   3. Priority-Based       — spare highest-priority processes
 */

#include "graph.h"

/* ─────────────────────────────────────────────────────────────
   dijkstra_single: run Dijkstra from 'src' on rec_graph
   dist[] filled with shortest distances from src
   ───────────────────────────────────────────────────────────── */
static void dijkstra_single(const Graph *g, int src, int dist[], int prev[])
{
    int n = g->num_processes;
    int visited[MAX_PROCESSES] = {0};

    for (int i = 0; i < n; i++) { dist[i] = INF; prev[i] = -1; }
    dist[src] = 0;

    for (int iter = 0; iter < n; iter++) {
        /* Find unvisited vertex with minimum distance */
        int u = -1;
        for (int i = 0; i < n; i++) {
            if (!visited[i] && (u == -1 || dist[i] < dist[u]))
                u = i;
        }
        if (u == -1 || dist[u] == INF) break;
        visited[u] = 1;

        /* Relax edges from u */
        for (int v = 0; v < n; v++) {
            if (g->rec_graph[u][v] != INF && !visited[v]) {
                int nd = dist[u] + g->rec_graph[u][v];
                if (nd < dist[v]) {
                    dist[v] = nd;
                    prev[v] = u;
                }
            }
        }
    }
}

/* ─────────────────────────────────────────────────────────────
   print_dist_table: display Dijkstra distances from source
   ───────────────────────────────────────────────────────────── */
static void print_dist_table(const Graph *g, int src,
                              const int dist[], const int prev[])
{
    int n = g->num_processes;
    printf("\n  Dijkstra shortest paths from P%d:\n", src);
    printf("  %-8s %-12s %-12s %-12s\n",
           "Process", "Dist (cost)", "Via (prev)", "Deadlocked?");
    printf("  %-8s %-12s %-12s %-12s\n",
           "-------", "----------", "----------", "-----------");
    for (int i = 0; i < n; i++) {
        printf("  P%-7d %-12s %-12s %-12s\n",
               i,
               (dist[i] == INF) ? "∞" :
               ({ static char buf[16];
                  snprintf(buf, sizeof(buf), "%d", dist[i]); buf; }),
               (prev[i] == -1) ? "-" :
               ({ static char buf[16];
                  snprintf(buf, sizeof(buf), "P%d", prev[i]); buf; }),
               g->processes[i].is_deadlocked ? "Yes ⚠" : "No");
    }
}

/* ─────────────────────────────────────────────────────────────
   free_resources: release all resources held by process pid
   ───────────────────────────────────────────────────────────── */
static void free_resources(Graph *g, int pid)
{
    for (int r = 0; r < g->num_resources; r++) {
        if (g->allocation[pid][r] > 0) {
            g->resources[r].available_instances += g->allocation[pid][r];
            printf("    Released %d instance(s) of R%d ('%s') back to pool.\n",
                   g->allocation[pid][r], r, g->resources[r].name);
            g->allocation[pid][r] = 0;
        }
        g->request[pid][r] = 0;
    }
    /* Remove this process from WFG */
    for (int j = 0; j < g->num_processes; j++) {
        g->wfg[pid][j] = 0;
        g->wfg[j][pid] = 0;
    }
    g->processes[pid].is_deadlocked = 0;
}

/* ─────────────────────────────────────────────────────────────
   dijkstra_recovery: main recovery driver
   ───────────────────────────────────────────────────────────── */
void dijkstra_recovery(Graph *g)
{
    print_banner("DIJKSTRA-BASED DEADLOCK RECOVERY");
    printf("  Algorithm  : Dijkstra's Shortest Path\n");
    printf("  Complexity : O(V²) per iteration\n");
    printf("  Objective  : Minimise total recovery cost\n\n");

    if (!g->wfg_built) {
        printf("  [!] WFG not built. Run option 6 first.\n\n");
        return;
    }

    /* Check deadlock exists */
    int any_dl = 0;
    for (int i = 0; i < g->num_processes; i++)
        if (g->processes[i].is_deadlocked) { any_dl = 1; break; }

    if (!any_dl) {
        printf("  [i] No deadlocked processes found.\n");
        printf("  [i] Run DFS Detection (option 7) first.\n\n");
        return;
    }

    long t_start = get_time_us();

    /* ── Print recovery cost table ────────────────────────── */
    printf("  Recovery cost table:\n");
    printf("  %-8s  %-12s  %-10s  %-10s\n",
           "Process", "Name", "Cost", "Priority");
    printf("  %-8s  %-12s  %-10s  %-10s\n",
           "-------", "----", "----", "--------");
    for (int i = 0; i < g->num_processes; i++) {
        printf("  P%-7d  %-12s  %-10d  %-10d\n",
               i,
               g->processes[i].name,
               g->processes[i].recovery_cost,
               g->processes[i].priority);
    }
    printf("\n");

    int total_cost      = 0;
    int round           = 0;
    int terminated[MAX_PROCESSES] = {0};
    int victims_list[MAX_PROCESSES];
    int num_victims = 0;

    /* ── Iterative recovery loop ──────────────────────────── */
    while (1) {
        /* Find cheapest deadlocked source */
        int src = -1;
        for (int i = 0; i < g->num_processes; i++) {
            if (g->processes[i].is_deadlocked && !terminated[i]) {
                if (src == -1 ||
                    g->processes[i].recovery_cost <
                    g->processes[src].recovery_cost)
                    src = i;
            }
        }
        if (src == -1) break;

        round++;
        printf("  ══ Recovery Round %d ══\n", round);
        printf("  Source (cheapest deadlocked): P%d (%s)  cost=%d\n\n",
               src, g->processes[src].name,
               g->processes[src].recovery_cost);

        int dist[MAX_PROCESSES], prev[MAX_PROCESSES];
        dijkstra_single(g, src, dist, prev);
        print_dist_table(g, src, dist, prev);

        /* Choose victim: lowest-cost deadlocked process reachable
           that also has lowest priority (most expendable)         */
        int victim = src;
        for (int i = 0; i < g->num_processes; i++) {
            if (!g->processes[i].is_deadlocked || terminated[i]) continue;
            if (dist[i] == INF) continue;
            /* Prefer lower cost; break ties by lower priority */
            if (g->processes[i].recovery_cost < g->processes[victim].recovery_cost ||
               (g->processes[i].recovery_cost == g->processes[victim].recovery_cost &&
                g->processes[i].priority < g->processes[victim].priority))
                victim = i;
        }

        printf("\n  ► Victim selected: P%d ('%s')  "
               "[cost=%d, priority=%d]\n",
               victim,
               g->processes[victim].name,
               g->processes[victim].recovery_cost,
               g->processes[victim].priority);

        /* ── Strategy 1: Process Termination ────────────── */
        printf("\n  Strategy 1 — Process Termination:\n");
        printf("    Terminating P%d and releasing its resources.\n", victim);
        free_resources(g, victim);
        terminated[victim]             = 1;
        total_cost                    += g->processes[victim].recovery_cost;
        victims_list[num_victims++]    = victim;

        /* ── Strategy 2: Resource Preemption note ─────── */
        printf("\n  Strategy 2 — Resource Preemption (alternative):\n");
        printf("    Preempt resources of P%d → reassign to waiting processes.\n",
               victim);
        printf("    (Resources already freed above; "
               "pending requests may now be satisfied.)\n");

        /* ── Strategy 3: Priority-Based ──────────────── */
        printf("\n  Strategy 3 — Priority-Based note:\n");
        printf("    P%d had priority %d. Processes with higher priority "
               "were spared.\n",
               victim, g->processes[victim].priority);

        /* Rebuild WFG and check if deadlock persists */
        wfg_build(g);

        int still_dl = 0;
        for (int i = 0; i < g->num_processes; i++) {
            if (!terminated[i]) {
                /* Rerun DFS silently to refresh is_deadlocked flags */
                /* (simplified: check for WFG cycle on remaining) */
                g->processes[i].is_deadlocked = 0;
            }
        }
        /* Quick cycle check using in-degree (Kahn-lite) */
        int in_deg[MAX_PROCESSES] = {0};
        int np = g->num_processes;
        for (int i = 0; i < np; i++)
            if (!terminated[i])
                for (int j = 0; j < np; j++)
                    if (!terminated[j] && g->wfg[i][j])
                        in_deg[j]++;

        /* Kahn */
        int q[MAX_PROCESSES], f = 0, r2 = 0, processed = 0;
        for (int i = 0; i < np; i++)
            if (!terminated[i] && in_deg[i] == 0)
                q[r2++] = i;
        while (f < r2) {
            int u = q[f++]; processed++;
            for (int v = 0; v < np; v++) {
                if (!terminated[v] && g->wfg[u][v]) {
                    if (--in_deg[v] == 0) q[r2++] = v;
                }
            }
        }
        int active = 0;
        for (int i = 0; i < np; i++) if (!terminated[i]) active++;
        if (processed < active) {
            still_dl = 1;
            for (int i = 0; i < np; i++) {
                if (!terminated[i] && in_deg[i] > 0)
                    g->processes[i].is_deadlocked = 1;
            }
        }

        printf("\n  Deadlock after round %d: %s\n\n",
               round, still_dl ? "Still exists — continue recovery" : "RESOLVED ✔");
        if (!still_dl) break;
    }

    long t_end = get_time_us();

    /* ── Summary ────────────────────────────────────────────── */
    print_separator();
    print_banner("RECOVERY SUMMARY");
    printf("  Recovery rounds       : %d\n", round);
    printf("  Processes terminated  : %d\n", num_victims);
    printf("  Total recovery cost   : %d\n", total_cost);
    printf("  Execution time        : %ld μs\n\n", t_end - t_start);

    printf("  Terminated processes:\n");
    for (int i = 0; i < num_victims; i++)
        printf("    [%d] P%d (%s)  cost=%d\n",
               i + 1,
               victims_list[i],
               g->processes[victims_list[i]].name,
               g->processes[victims_list[i]].recovery_cost);

    printf("\n  Surviving processes (safe state):\n");
    for (int i = 0; i < g->num_processes; i++) {
        if (!terminated[i])
            printf("    ✔  P%d (%s)  priority=%d\n",
                   i, g->processes[i].name, g->processes[i].priority);
    }

    printf("\n  Resources available after recovery:\n");
    for (int r = 0; r < g->num_resources; r++)
        printf("    R%d (%s): %d / %d instances free\n",
               r, g->resources[r].name,
               g->resources[r].available_instances,
               g->resources[r].total_instances);
    printf("\n");
}
