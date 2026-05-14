/*
 * rag.c — Resource Allocation Graph (RAG) construction
 *
 * RAG edges:
 *   Process → Resource : Request edge  (process wants resource)
 *   Resource → Process : Allocation edge (resource is held by process)
 *
 * Internally we maintain two matrices:
 *   allocation[p][r] = number of R instances currently held by P
 *   request[p][r]    = number of R instances currently requested by P
 */

#include "graph.h"

/* ─────────────────────────────────────────────────────────────
   rag_allocate: assign 'instances' units of resource rid to process pid
   ───────────────────────────────────────────────────────────── */
int rag_allocate(Graph *g, int pid, int rid, int instances)
{
    if (pid < 0 || pid >= g->num_processes) {
        printf("  [!] Invalid process id: %d\n", pid); return -1;
    }
    if (rid < 0 || rid >= g->num_resources) {
        printf("  [!] Invalid resource id: %d\n", rid); return -1;
    }
    if (instances <= 0) {
        printf("  [!] Instance count must be > 0.\n"); return -1;
    }
    if (g->resources[rid].available_instances < instances) {
        printf("  [!] Not enough instances of R%d available "
               "(have %d, need %d).\n",
               rid,
               g->resources[rid].available_instances,
               instances);
        return -1;
    }
    g->allocation[pid][rid]              += instances;
    g->resources[rid].available_instances -= instances;
    printf("  [+] Allocated %d instance(s) of R%d to P%d  "
           "[R%d available: %d]\n",
           instances, rid, pid, rid,
           g->resources[rid].available_instances);
    return 0;
}

/* ─────────────────────────────────────────────────────────────
   rag_request: record that process pid is waiting for 'instances'
                units of resource rid
   ───────────────────────────────────────────────────────────── */
int rag_request(Graph *g, int pid, int rid, int instances)
{
    if (pid < 0 || pid >= g->num_processes) {
        printf("  [!] Invalid process id: %d\n", pid); return -1;
    }
    if (rid < 0 || rid >= g->num_resources) {
        printf("  [!] Invalid resource id: %d\n", rid); return -1;
    }
    if (instances <= 0) {
        printf("  [!] Instance count must be > 0.\n"); return -1;
    }
    g->request[pid][rid] += instances;
    printf("  [+] P%d is now requesting %d instance(s) of R%d\n",
           pid, instances, rid);
    return 0;
}

/* ─────────────────────────────────────────────────────────────
   rag_display: print allocation and request tables
   ───────────────────────────────────────────────────────────── */
void rag_display(const Graph *g)
{
    print_banner("RESOURCE ALLOCATION GRAPH (RAG)");

    if (g->num_processes == 0 || g->num_resources == 0) {
        printf("  [!] No processes or resources defined yet.\n\n");
        return;
    }

    /* Header */
    printf("  Allocation edges  (Resource → Process):\n");
    int any = 0;
    for (int r = 0; r < g->num_resources; r++) {
        for (int p = 0; p < g->num_processes; p++) {
            if (g->allocation[p][r] > 0) {
                printf("    R%d ──[%d]──▶ P%d\n",
                       r, g->allocation[p][r], p);
                any = 1;
            }
        }
    }
    if (!any) printf("    (none)\n");

    printf("\n  Request edges  (Process → Resource):\n");
    any = 0;
    for (int p = 0; p < g->num_processes; p++) {
        for (int r = 0; r < g->num_resources; r++) {
            if (g->request[p][r] > 0) {
                printf("    P%d ──[%d]──▶ R%d\n",
                       p, g->request[p][r], r);
                any = 1;
            }
        }
    }
    if (!any) printf("    (none)\n");
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
   rag_random: generate a random RAG with np processes, nr resources
   ───────────────────────────────────────────────────────────── */
void rag_random(Graph *g, int np, int nr)
{
    graph_init(g);
    srand((unsigned)time(NULL));

    if (np < 2)  np = 2;
    if (np > MAX_PROCESSES) np = MAX_PROCESSES;
    if (nr < 1)  nr = 1;
    if (nr > MAX_RESOURCES) nr = MAX_RESOURCES;

    char buf[MAX_NAME];

    /* Add processes */
    for (int i = 0; i < np; i++) {
        snprintf(buf, sizeof(buf), "Proc%d", i);
        graph_add_process(g, buf,
                          (rand() % 9) + 1,   /* cost 1-9  */
                          (rand() % 5) + 1);  /* pri  1-5  */
    }

    /* Add resources */
    for (int i = 0; i < nr; i++) {
        snprintf(buf, sizeof(buf), "Res%d", i);
        graph_add_resource(g, buf, (rand() % 3) + 1); /* 1-3 instances */
    }

    /* Random allocations */
    for (int p = 0; p < np; p++) {
        int r = rand() % nr;
        if (g->resources[r].available_instances > 0)
            rag_allocate(g, p, r, 1);
    }

    /* Random requests — create at least one deadlock-prone cycle */
    for (int p = 0; p < np; p++) {
        int r = rand() % nr;
        /* request a resource held by the next process → ring */
        if (g->allocation[(p + 1) % np][r] > 0)
            rag_request(g, p, r, 1);
    }

    printf("\n  [i] Random RAG generated: %d processes, %d resources.\n\n",
           np, nr);
}

/* ─────────────────────────────────────────────────────────────
   rag_test_case: load a predefined deadlock scenario
   ───────────────────────────────────────────────────────────── */
void rag_test_case(Graph *g, int tc)
{
    graph_init(g);

    switch (tc) {

    /* ── Test Case 1: Classic 3-process cycle ─────────────── */
    case 1:
        printf("\n  [TC1] Classic 3-process deadlock: P0→P1→P2→P0\n\n");
        graph_add_process(g, "Alpha",  4, 3);
        graph_add_process(g, "Beta",   2, 5);
        graph_add_process(g, "Gamma",  7, 2);
        graph_add_resource(g, "CPU",   1);
        graph_add_resource(g, "Disk",  1);
        graph_add_resource(g, "Net",   1);

        rag_allocate(g, 0, 0, 1);   /* P0 holds CPU  */
        rag_allocate(g, 1, 1, 1);   /* P1 holds Disk */
        rag_allocate(g, 2, 2, 1);   /* P2 holds Net  */

        rag_request(g, 0, 1, 1);    /* P0 wants Disk (held by P1) */
        rag_request(g, 1, 2, 1);    /* P1 wants Net  (held by P2) */
        rag_request(g, 2, 0, 1);    /* P2 wants CPU  (held by P0) */
        break;

    /* ── Test Case 2: 5-process partial deadlock ──────────── */
    case 2:
        printf("\n  [TC2] 5-process partial deadlock: P0↔P1 cycle; "
               "P2,P3,P4 safe\n\n");
        graph_add_process(g, "P_A", 3, 4);
        graph_add_process(g, "P_B", 5, 2);
        graph_add_process(g, "P_C", 1, 5);
        graph_add_process(g, "P_D", 6, 1);
        graph_add_process(g, "P_E", 2, 3);
        graph_add_resource(g, "R1", 1);
        graph_add_resource(g, "R2", 1);
        graph_add_resource(g, "R3", 2);

        rag_allocate(g, 0, 0, 1);
        rag_allocate(g, 1, 1, 1);
        rag_allocate(g, 2, 2, 1);

        rag_request(g, 0, 1, 1);    /* P0 ↔ P1 deadlock */
        rag_request(g, 1, 0, 1);
        rag_request(g, 2, 2, 1);    /* P2 waits for extra R3 instance — */
                                    /* but R3 has 2 total, 1 free → safe */
        break;

    /* ── Test Case 3: 4-process chain + cross edge ────────── */
    case 3:
        printf("\n  [TC3] 4-process chain: P0→P1→P2→P3→P0  "
               "(full ring)\n\n");
        graph_add_process(g, "Proc0", 5, 3);
        graph_add_process(g, "Proc1", 3, 4);
        graph_add_process(g, "Proc2", 8, 1);
        graph_add_process(g, "Proc3", 2, 5);
        graph_add_resource(g, "Mem",  1);
        graph_add_resource(g, "IO",   1);
        graph_add_resource(g, "GPU",  1);
        graph_add_resource(g, "Bus",  1);

        rag_allocate(g, 0, 0, 1);   /* P0: Mem  */
        rag_allocate(g, 1, 1, 1);   /* P1: IO   */
        rag_allocate(g, 2, 2, 1);   /* P2: GPU  */
        rag_allocate(g, 3, 3, 1);   /* P3: Bus  */

        rag_request(g, 0, 1, 1);    /* P0 → P1 */
        rag_request(g, 1, 2, 1);    /* P1 → P2 */
        rag_request(g, 2, 3, 1);    /* P2 → P3 */
        rag_request(g, 3, 0, 1);    /* P3 → P0 */
        break;

    default:
        printf("  [!] Unknown test case: %d. Choose 1-3.\n\n", tc);
        break;
    }
}
