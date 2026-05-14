/*
 * main.c — Algorithmic Detection and Resolution of Deadlocks
 *          in Resource Allocation Graphs
 *
 * Design and Analysis of Algorithms — Term Project
 *
 * Menu-driven simulator implementing:
 *   • DFS-based deadlock detection     O(V+E)
 *   • Floyd–Warshall verification      O(V³)
 *   • Topological sort check           O(V+E)
 *   • BFS dependency traversal         O(V+E)
 *   • Dijkstra optimal recovery        O(V²)
 *
 * Build:  make
 * Run:    ./deadlock_sim
 */

#include "graph.h"

/* ═══════════════════════════════════════════════════════════════
   FORWARD DECLARATIONS
   ═══════════════════════════════════════════════════════════════ */
static void print_main_menu(void);
static void handle_add_process(Graph *g);
static void handle_add_resource(Graph *g);
static void handle_allocate(Graph *g);
static void handle_request(Graph *g);
static void handle_bfs_single(Graph *g);
static void handle_final_state(const Graph *g);
static void splash_screen(void);
static void flush_input(void);
static int  safe_int(const char *prompt, int lo, int hi);

/* ═══════════════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════════════ */
int main(void)
{
    Graph g;
    graph_init(&g);

    splash_screen();

    int choice = 0;

    while (1) {
        print_main_menu();
        printf("  Enter choice: ");
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        switch (choice) {

        /* ── 1. Add Process ────────────────────────────────── */
        case 1:
            handle_add_process(&g);
            break;

        /* ── 2. Add Resource ───────────────────────────────── */
        case 2:
            handle_add_resource(&g);
            break;

        /* ── 3. Allocate Resource ──────────────────────────── */
        case 3:
            handle_allocate(&g);
            break;

        /* ── 4. Request Resource ───────────────────────────── */
        case 4:
            handle_request(&g);
            break;

        /* ── 5. Build / Display RAG ────────────────────────── */
        case 5:
            rag_display(&g);
            ascii_rag(&g);
            break;

        /* ── 6. Convert RAG → WFG ──────────────────────────── */
        case 6:
            graph_reset_state(&g);
            wfg_build(&g);
            wfg_display(&g);
            ascii_wfg(&g);
            break;

        /* ── 7. DFS Deadlock Detection ─────────────────────── */
        case 7:
            dfs_detect_deadlock(&g);
            break;

        /* ── 8. Floyd–Warshall Verification ────────────────── */
        case 8:
            floyd_warshall(&g);
            break;

        /* ── 9. Topological Sort Check ─────────────────────── */
        case 9:
            topo_sort_check(&g);
            break;

        /* ── 10. BFS Traversal ─────────────────────────────── */
        case 10:
            handle_bfs_single(&g);
            break;

        /* ── 11. Dijkstra Recovery ─────────────────────────── */
        case 11:
            dijkstra_recovery(&g);
            break;

        /* ── 12. Display Final Safe State ──────────────────── */
        case 12:
            handle_final_state(&g);
            break;

        /* ── 13. Performance Comparison ────────────────────── */
        case 13:
            if (!g.wfg_built) {
                printf("\n  [!] Build WFG first (option 6).\n\n");
            } else {
                performance_report(&g);
            }
            break;

        /* ── 14. Generate Graphviz File ────────────────────── */
        case 14:
            export_graphviz(&g, "outputs/rag.dot");
            break;

        /* ── 15. Display Current System State ──────────────── */
        case 15:
            graph_display(&g);
            break;

        /* ── 16. Load Predefined Test Case ─────────────────── */
        case 16: {
            int tc = safe_int("  Test case number (1=3-proc, 2=5-proc, "
                              "3=4-proc ring): ", 1, 3);
            rag_test_case(&g, tc);
            printf("\n  [i] Test case loaded. Run option 6 to build WFG.\n\n");
            break;
        }

        /* ── 17. Random Graph Generation ───────────────────── */
        case 17: {
            int np = safe_int("  Number of processes (2-10): ", 2, 10);
            int nr = safe_int("  Number of resources (1-6):  ", 1, 6);
            rag_random(&g, np, nr);
            printf("\n  [i] Random graph loaded. Run option 6 to build WFG.\n\n");
            break;
        }

        /* ── 18. Run Full Pipeline ──────────────────────────── */
        case 18:
            print_banner("FULL DETECTION PIPELINE");
            printf("  Running complete workflow automatically...\n\n");

            /* Step 1: Build WFG */
            graph_reset_state(&g);
            wfg_build(&g);

            /* Step 2: DFS */
            printf("\n"); print_separator();
            dfs_detect_deadlock(&g);

            /* Step 3: Floyd–Warshall */
            print_separator();
            floyd_warshall(&g);

            /* Step 4: Topo Sort */
            print_separator();
            topo_sort_check(&g);

            /* Step 5: BFS from process 0 */
            print_separator();
            if (g.num_processes > 0)
                bfs_traversal(&g, 0);

            /* Step 6: Dijkstra Recovery (if deadlock) */
            if (g.deadlock_found) {
                print_separator();
                dijkstra_recovery(&g);
            }

            /* Step 7: Final state */
            print_separator();
            handle_final_state(&g);

            /* Step 8: Export */
            export_graphviz(&g, "outputs/rag.dot");
            break;

        /* ── 0. Exit ────────────────────────────────────────── */
        case 0:
            print_banner("EXITING SIMULATOR");
            printf("  Thank you for using the Deadlock Simulator.\n");
            printf("  DAA Project — Resource Allocation Graph Analysis\n\n");
            return 0;

        default:
            printf("\n  [!] Invalid choice. Please enter 0-18.\n\n");
            break;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════
   HELPERS
   ═══════════════════════════════════════════════════════════════ */

static void splash_screen(void)
{
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════╗\n");
    printf("  ║                                                      ║\n");
    printf("  ║   DEADLOCK DETECTION AND RECOVERY SIMULATOR          ║\n");
    printf("  ║   Resource Allocation Graph analysis in C            ║\n");
    printf("  ║   Algorithms: DFS, FW, Topo, BFS, Dijkstra           ║\n");
    printf("  ║                                                      ║\n");
    printf("  ╚══════════════════════════════════════════════════════╝\n\n");
    printf("  Quick start:\n");
    printf("    Option 16 → load a test case\n");
    printf("    Option 18 → run full pipeline automatically\n\n");
}

static void print_main_menu(void)
{
    printf("  ┌──────────────────────────────────────────────────────┐\n");
    printf("  │                       MAIN MENU                      │\n");
    printf("  ├──────────────────────────────────────────────────────┤\n");
    printf("  │  1.  Add Process                                     │\n");
    printf("  │  2.  Add Resource                                    │\n");
    printf("  │  3.  Allocate Resource                               │\n");
    printf("  │  4.  Request Resource                                │\n");
    printf("  │  5.  Display RAG                                     │\n");
    printf("  │  6.  Build WFG                                       │\n");
    printf("  │  7.  DFS Detection                                   │\n");
    printf("  │  8.  Floyd-Warshall                                  │\n");
    printf("  │  9.  Topo Sort Check                                 │\n");
    printf("  │ 10.  BFS Traversal (single)                          │\n");
    printf("  │ 11.  Dijkstra Recovery                               │\n");
    printf("  │ 12.  Show Final Safe State                           │\n");
    printf("  │ 13.  Performance Comparison                          │\n");
    printf("  │ 14.  Export Graphviz (.dot)                          │\n");
    printf("  │ 15.  Display System State                            │\n");
    printf("  │ 16.  Load Test Case (1/2/3)                          │\n");
    printf("  │ 17.  Random Graph                                    │\n");
    printf("  │ 18.  Run Full Pipeline                               │\n");
    printf("  ├──────────────────────────────────────────────────────┤\n");
    printf("  │  0. Exit                                             │\n");
    printf("  └──────────────────────────────────────────────────────┘\n");
}

static void flush_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static int safe_int(const char *prompt, int lo, int hi)
{
    int v;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &v) == 1 && v >= lo && v <= hi) {
            flush_input();
            return v;
        }
        flush_input();
        printf("  [!] Please enter a value between %d and %d.\n", lo, hi);
    }
}

static void handle_add_process(Graph *g)
{
    char name[MAX_NAME];
    printf("\n  Process name (e.g. P_Alpha): ");
    if (scanf("%15s", name) != 1) { flush_input(); return; }
    flush_input();
    int cost = safe_int("  Recovery cost (1-99): ", 1, 99);
    int prio = safe_int("  Priority (1-10, higher=more critical): ", 1, 10);
    printf("\n");
    graph_add_process(g, name, cost, prio);
    printf("\n");
}

static void handle_add_resource(Graph *g)
{
    char name[MAX_NAME];
    printf("\n  Resource name (e.g. CPU): ");
    if (scanf("%15s", name) != 1) { flush_input(); return; }
    flush_input();
    int inst = safe_int("  Total instances (1-10): ", 1, 10);
    printf("\n");
    graph_add_resource(g, name, inst);
    printf("\n");
}

static void handle_allocate(Graph *g)
{
    if (g->num_processes == 0 || g->num_resources == 0) {
        printf("\n  [!] Add processes and resources first.\n\n");
        return;
    }
    graph_display(g);
    int pid = safe_int("  Process ID to allocate TO: ", 0, g->num_processes - 1);
    int rid = safe_int("  Resource ID to allocate:   ", 0, g->num_resources - 1);
    int ins = safe_int("  Number of instances:       ", 1, 10);
    printf("\n");
    rag_allocate(g, pid, rid, ins);
    printf("\n");
}

static void handle_request(Graph *g)
{
    if (g->num_processes == 0 || g->num_resources == 0) {
        printf("\n  [!] Add processes and resources first.\n\n");
        return;
    }
    graph_display(g);
    int pid = safe_int("  Process ID making request: ", 0, g->num_processes - 1);
    int rid = safe_int("  Resource ID requested:     ", 0, g->num_resources - 1);
    int ins = safe_int("  Number of instances:       ", 1, 10);
    printf("\n");
    rag_request(g, pid, rid, ins);
    printf("\n");
}

static void handle_bfs_single(Graph *g)
{
    if (!g->wfg_built) {
        printf("\n  [!] WFG not built. Run option 6 first.\n\n");
        return;
    }
    printf("\n  Available processes: 0 – %d\n", g->num_processes - 1);
    int pid = safe_int("  Starting process ID for BFS: ",
                       0, g->num_processes - 1);
    printf("\n");
    print_banner("BFS DEPENDENCY TRAVERSAL");
    printf("  Algorithm : Breadth-First Search on WFG\n");
    printf("  Complexity : O(V + E)\n\n");
    bfs_traversal(g, pid);
}

static void handle_final_state(const Graph *g)
{
    print_banner("FINAL SAFE STATE");

    int any_dl = 0;
    for (int i = 0; i < g->num_processes; i++)
        if (g->processes[i].is_deadlocked) { any_dl = 1; break; }

    if (!any_dl) {
        printf("  ✔  System is in a SAFE STATE.\n");
        printf("  ✔  No deadlocked processes.\n\n");
    } else {
        printf("  ⚠  System is in an UNSAFE STATE.\n\n");
        printf("  Deadlocked processes:\n");
        for (int i = 0; i < g->num_processes; i++) {
            if (g->processes[i].is_deadlocked)
                printf("    ✗ P%d (%s)  [recovery cost: %d]\n",
                       i, g->processes[i].name,
                       g->processes[i].recovery_cost);
        }
        printf("\n  Recommendation: Run Dijkstra Recovery (option 11).\n\n");
    }

    printf("  Resource availability:\n");
    for (int r = 0; r < g->num_resources; r++) {
        int avail = g->resources[r].available_instances;
        int total = g->resources[r].total_instances;
        char bar[22]; int fill = (avail * 20) / (total ? total : 1);
        for (int i = 0; i < 20; i++) bar[i] = (i < fill) ? '#' : '-';
        bar[20] = '\0';
        printf("    R%d (%s):  [%s]  %d/%d free\n",
               r, g->resources[r].name, bar, avail, total);
    }
    printf("\n");
}
