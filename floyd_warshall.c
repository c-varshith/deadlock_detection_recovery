/*
 * floyd_warshall.c — Floyd–Warshall Transitive Closure Verification
 *
 * Purpose:
 *   Use Floyd–Warshall to compute the reachability (transitive closure)
 *   of the Wait-For Graph.  If reach[i][i] = 1 after the algorithm, then
 *   process Pi can reach itself through the wait-for chain — a cycle exists.
 *
 * Algorithm (transitive closure variant):
 *   Initialise: reach[i][j] = wfg[i][j] (or 1 if i==j for self-loops)
 *   For k = 0 to V-1:
 *     For i = 0 to V-1:
 *       For j = 0 to V-1:
 *         reach[i][j] = reach[i][j] OR (reach[i][k] AND reach[k][j])
 *
 * Complexity:
 *   Time : O(V³)
 *   Space: O(V²)
 *
 * Comparison with DFS:
 *   DFS is faster (O(V+E)) for single deadlock detection, but
 *   Floyd–Warshall gives the full reachability picture — useful for
 *   identifying ALL pairs of mutually-waiting processes simultaneously.
 */

#include "graph.h"

/* ─────────────────────────────────────────────────────────────
   floyd_warshall: compute transitive closure and detect cycles
   ───────────────────────────────────────────────────────────── */
void floyd_warshall(const Graph *g)
{
    print_banner("FLOYD–WARSHALL DEADLOCK VERIFICATION");
    printf("  Algorithm : Floyd–Warshall Transitive Closure\n");
    printf("  Complexity : Time O(V³)  |  Space O(V²)\n");
    printf("  V = %d processes\n\n", g->num_processes);

    if (!g->wfg_built) {
        printf("  [!] WFG not built. Run option 6 first.\n\n");
        return;
    }

    int n = g->num_processes;
    if (n == 0) { printf("  (no processes)\n\n"); return; }

    long t_start = get_time_us();

    /* ── Step 1: Initialise reach matrix from WFG ─────────── */
    int reach[MAX_PROCESSES][MAX_PROCESSES];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            reach[i][j] = g->wfg[i][j];
        }
        /* Do NOT set diagonal to 1 here — self-loop after closure
           indicates a genuine cycle, not a trivial self-reference. */
    }

    print_matrix_np("Initial Adjacency / Reach Matrix", reach, n);

    /* ── Step 2: Floyd–Warshall relaxation ───────────────────
       reach[i][j] = 1 if there is a directed path i → … → j  */
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (reach[i][k] && reach[k][j])
                    reach[i][j] = 1;
            }
        }
    }

    print_matrix_np("Final Reachability (Transitive Closure) Matrix", reach, n);

    long t_end = get_time_us();

    /* ── Step 3: Analyse diagonal ────────────────────────────
       reach[i][i] = 1  ⟹  Pi can reach itself ⟹ Pi in a cycle */
    printf("  Cycle (self-reachability) analysis:\n");
    printf("  %-8s  %-14s  %-10s\n", "Process", "reach[i][i]", "In Cycle?");
    printf("  %-8s  %-14s  %-10s\n", "-------", "-----------", "--------");

    int found = 0;
    for (int i = 0; i < n; i++) {
        int in_cycle = reach[i][i];
        printf("  P%-7d  %-14d  %s\n",
               i, in_cycle, in_cycle ? "YES ⚠" : "No");
        if (in_cycle) found = 1;
    }

    printf("\n");
    if (found) {
        printf("  ╔══════════════════════════════════════════════╗\n");
        printf("  ║   ⚠   DEADLOCK CONFIRMED by Floyd–Warshall  ║\n");
        printf("  ╚══════════════════════════════════════════════╝\n");

        /* Also show all mutually-reachable pairs */
        printf("\n  Reachability pairs (Pi → Pj, Pi ≠ Pj):\n");
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (i != j && reach[i][j])
                    printf("    P%d  ──reachable──▶  P%d\n", i, j);
    } else {
        printf("  ✔  No deadlock confirmed by Floyd–Warshall.\n");
    }

    printf("\n  Execution time : %ld μs\n\n", t_end - t_start);
}
