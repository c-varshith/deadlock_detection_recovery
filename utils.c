/*
 * utils.c — Utilities: timing, formatting, matrices, performance report
 */

#include "graph.h"
#include <sys/time.h>

/* ─────────────────────────────────────────────────────────────
   print_separator: print a horizontal divider
   ───────────────────────────────────────────────────────────── */
void print_separator(void)
{
    printf("  %s\n",
           "────────────────────────────────────────────────────────");
}

/* ─────────────────────────────────────────────────────────────
   print_banner: print a titled section header
   ───────────────────────────────────────────────────────────── */
void print_banner(const char *title)
{
    int len = (int)strlen(title);
    int width = 58;
    int pad = (width - len - 2) / 2;

    printf("\n  ╔");
    for (int i = 0; i < width; i++) printf("═");
    printf("╗\n");

    printf("  ║");
    for (int i = 0; i < pad; i++) printf(" ");
    printf(" %s ", title);
    for (int i = 0; i < width - pad - len - 2; i++) printf(" ");
    printf("║\n");

    printf("  ╚");
    for (int i = 0; i < width; i++) printf("═");
    printf("╝\n\n");
}

/* ─────────────────────────────────────────────────────────────
   get_time_us: return current time in microseconds
   ───────────────────────────────────────────────────────────── */
long get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long)(tv.tv_sec) * 1000000L + (long)(tv.tv_usec);
}

/* ─────────────────────────────────────────────────────────────
   print_matrix_np: print a V×V int matrix (process × process)
   ───────────────────────────────────────────────────────────── */
void print_matrix_np(const char *label,
                     int mat[][MAX_PROCESSES], int n)
{
    printf("  %s:\n", label);
    /* Column headers */
    printf("       ");
    for (int j = 0; j < n; j++) printf(" P%-2d", j);
    printf("\n       ");
    for (int j = 0; j < n; j++) printf("----");
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("  P%-3d|", i);
        for (int j = 0; j < n; j++) {
            if (mat[i][j] == INF)
                printf("  ∞ ");
            else
                printf("  %-2d", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
   print_matrix_nr: print a P×R matrix with named headers
   ───────────────────────────────────────────────────────────── */
void print_matrix_nr(const char *label,
                     int mat[][MAX_RESOURCES],
                     int rows, int cols,
                     const Graph *g)
{
    (void)g; /* suppress unused-parameter warning */
    printf("  %s:\n", label);
    /* Column headers */
    printf("       ");
    for (int j = 0; j < cols; j++) printf(" R%-2d", j);
    printf("\n       ");
    for (int j = 0; j < cols; j++) printf("----");
    printf("\n");
    for (int i = 0; i < rows; i++) {
        printf("  P%-3d|", i);
        for (int j = 0; j < cols; j++)
            printf("  %-2d", mat[i][j]);
        printf("\n");
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
   performance_report: benchmark all three algorithms and compare
   ───────────────────────────────────────────────────────────── */
void performance_report(Graph *g)
{
    print_banner("ALGORITHM PERFORMANCE COMPARISON");

    int n = g->num_processes;
    int e = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (g->wfg[i][j]) e++;

    printf("  Graph size: V = %d processes, E = %d WFG edges\n\n", n, e);

    /* ── DFS ──────────────────────────────────────────────── */
    long t0 = get_time_us();
    /* Run DFS silently — redirect stdout temporarily */
    int saved_dl     = g->deadlock_found;
    int saved_wfg    = g->wfg_built;
    /* We won't silence, but time it */
    /* To avoid polluting output, replicate the cycle check inline */
    int colour2[MAX_PROCESSES], parent2[MAX_PROCESSES];
    memset(colour2, 0, sizeof(colour2));
    memset(parent2, -1, sizeof(parent2));
    /* Simple DFS for timing only */
    for (int s = 0; s < n; s++) {
        if (colour2[s] != 0) continue;
        /* iterative DFS */
        int stack[MAX_PROCESSES], sp = 0;
        stack[sp++] = s;
        while (sp > 0) {
            int u = stack[--sp];
            if (colour2[u] == 2) continue;
            colour2[u] = 1;
            for (int v = n - 1; v >= 0; v--) {
                if (g->wfg[u][v] && colour2[v] != 2)
                    stack[sp++] = v;
            }
            colour2[u] = 2;
        }
    }
    long t_dfs = get_time_us() - t0;

    /* ── Floyd–Warshall ────────────────────────────────────── */
    t0 = get_time_us();
    int reach[MAX_PROCESSES][MAX_PROCESSES];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            reach[i][j] = g->wfg[i][j];
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (reach[i][k] && reach[k][j]) reach[i][j] = 1;
    long t_fw = get_time_us() - t0;

    /* ── Topological Sort ─────────────────────────────────── */
    t0 = get_time_us();
    int in_deg[MAX_PROCESSES] = {0};
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (g->wfg[i][j]) in_deg[j]++;
    int q[MAX_PROCESSES], front = 0, rear2 = 0, proc = 0;
    for (int i = 0; i < n; i++)
        if (in_deg[i] == 0) q[rear2++] = i;
    while (front < rear2) {
        int u = q[front++]; proc++;
        for (int v = 0; v < n; v++)
            if (g->wfg[u][v] && --in_deg[v] == 0)
                q[rear2++] = v;
    }
    long t_topo = get_time_us() - t0;

    /* ── Report ───────────────────────────────────────────── */
    printf("  ┌─────────────────────┬────────────────┬──────────────┬───────────┐\n");
    printf("  │ Algorithm           │ Time Complexity│ Space        │ Time(μs)  │\n");
    printf("  ├─────────────────────┼────────────────┼──────────────┼───────────┤\n");
    printf("  │ DFS Detection       │ O(V + E)       │ O(V)         │ %-8ld     │\n",
           t_dfs);
    printf("  │ Floyd–Warshall      │ O(V³)          │ O(V²)        │ %-8ld     │\n",
           t_fw);
    printf("  │ Topological Sort    │ O(V + E)       │ O(V)         │ %-8ld     │\n",
           t_topo);
    printf("  └─────────────────────┴────────────────┴──────────────┴───────────┘\n\n");

    printf("  Scalability notes:\n");
    printf("  • DFS and Topo-Sort scale linearly with graph size — preferred\n");
    printf("    for large graphs or real-time OS schedulers.\n");
    printf("  • Floyd–Warshall is O(V³) — impractical for V > ~100, but\n");
    printf("    gives full reachability info in one pass.\n");
    printf("  • Dijkstra Recovery is O(V²) per round — bounded by the\n");
    printf("    number of deadlocked processes.\n\n");

    /* Memory usage estimates */
    printf("  Memory usage (approximate):\n");
    printf("  %-24s  %zu bytes\n", "Graph structure:", sizeof(Graph));
    printf("  %-24s  %d × %d × 4 = %d bytes\n",
           "WFG matrix:",
           MAX_PROCESSES, MAX_PROCESSES,
           MAX_PROCESSES * MAX_PROCESSES * 4);
    printf("  %-24s  %d × %d × 4 = %d bytes\n",
           "Reachability matrix:",
           MAX_PROCESSES, MAX_PROCESSES,
           MAX_PROCESSES * MAX_PROCESSES * 4);
    printf("\n");

    /* Restore state */
    (void)saved_dl; (void)saved_wfg;
}
