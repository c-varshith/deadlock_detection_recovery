/*
 * wfg.c — Wait-For Graph (WFG) construction
 *
 * Algorithm:
 *   For each pair (Pi, Rk, Pj) where:
 *     Pi requests Rk  (request[i][k] > 0)
 *     Pj holds   Rk  (allocation[j][k] > 0)
 *   Add edge Pi → Pj in the WFG.
 *
 * Complexity: O(P² × R)
 *
 * The WFG eliminates resource nodes; it directly shows
 * process-level waiting dependencies.
 */

#include "graph.h"

/* ─────────────────────────────────────────────────────────────
   wfg_build: derive WFG from current RAG matrices
   ───────────────────────────────────────────────────────────── */
void wfg_build(Graph *g)
{
    print_banner("BUILDING WAIT-FOR GRAPH (WFG)");

    /* Clear previous WFG */
    memset(g->wfg, 0, sizeof(g->wfg));
    g->wfg_built = 0;

    if (g->num_processes == 0) {
        printf("  [!] No processes defined.\n\n");
        return;
    }

    int edges_added = 0;

    for (int i = 0; i < g->num_processes; i++) {        /* Pi requests */
        for (int k = 0; k < g->num_resources; k++) {    /* resource Rk */
            if (g->request[i][k] == 0) continue;

            for (int j = 0; j < g->num_processes; j++) { /* Pj holds Rk */
                if (i == j) continue;
                if (g->allocation[j][k] > 0) {
                    if (g->wfg[i][j] == 0) {             /* avoid duplicates */
                        g->wfg[i][j] = 1;
                        printf("  P%d waits for P%d  (via R%d: '%s')\n",
                               i, j, k, g->resources[k].name);
                        edges_added++;
                    }
                }
            }
        }
    }

    if (edges_added == 0)
        printf("  (No wait-for edges found — no circular waiting detected "
               "at construction)\n");

    g->wfg_built = 1;
    printf("\n  [i] WFG built with %d edge(s).\n\n", edges_added);

    /* Build recovery-cost graph as well */
    wfg_build_rec_graph(g);
}

/* ─────────────────────────────────────────────────────────────
   wfg_display: pretty-print the WFG adjacency matrix
   ───────────────────────────────────────────────────────────── */
void wfg_display(const Graph *g)
{
    print_banner("WAIT-FOR GRAPH — ADJACENCY MATRIX");

    if (!g->wfg_built) {
        printf("  [!] WFG not yet built. Run option 6 first.\n\n");
        return;
    }

    int n = g->num_processes;
    if (n == 0) { printf("  (empty)\n\n"); return; }

    /* Column headers */
    printf("  WFG[i][j] = 1 means Pi waits for Pj\n\n");
    printf("       ");
    for (int j = 0; j < n; j++) printf("  P%-2d", j);
    printf("\n       ");
    for (int j = 0; j < n; j++) printf("-----");
    printf("\n");

    for (int i = 0; i < n; i++) {
        printf("  P%-3d |", i);
        for (int j = 0; j < n; j++) {
            printf("  %s  ", g->wfg[i][j] ? "1" : ".");
        }
        printf("\n");
    }
    printf("\n");

    /* Also display as edge list */
    printf("  Edge list:\n");
    int any = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (g->wfg[i][j]) {
                printf("    P%d ──▶ P%d\n", i, j);
                any = 1;
            }
        }
    }
    if (!any) printf("    (no edges)\n");
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
   wfg_build_rec_graph: construct the weighted recovery graph.
   Edge weight between Pi and Pj = sum of their recovery costs.
   Used by Dijkstra to find minimum-cost termination sequence.
   ───────────────────────────────────────────────────────────── */
void wfg_build_rec_graph(Graph *g)
{
    int n = g->num_processes;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                g->rec_graph[i][j] = 0;
            } else if (g->wfg[i][j]) {
                /* Weight = cost of terminating process j (the holder) */
                g->rec_graph[i][j] = g->processes[j].recovery_cost;
            } else {
                g->rec_graph[i][j] = INF;
            }
        }
    }
}
