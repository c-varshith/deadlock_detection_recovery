/*
 * graph.c — Core graph initialization and entity management
 *
 * Responsibilities:
 *   - Initialize/reset the graph structure
 *   - Add processes and resources
 *   - Display current system state
 */

#include "graph.h"

/* ─────────────────────────────────────────────────────────────
   graph_init: zero-fill the entire graph structure
   ───────────────────────────────────────────────────────────── */
void graph_init(Graph *g)
{
    memset(g, 0, sizeof(Graph));
}

/* ─────────────────────────────────────────────────────────────
   graph_add_process: register a new process
   Returns: process id (0-based), or -1 on failure
   ───────────────────────────────────────────────────────────── */
int graph_add_process(Graph *g, const char *name, int cost, int priority)
{
    if (g->num_processes >= MAX_PROCESSES) {
        printf("  [!] Error: Maximum process limit (%d) reached.\n",
               MAX_PROCESSES);
        return -1;
    }
    int id = g->num_processes;
    g->processes[id].id            = id;
    g->processes[id].recovery_cost = (cost     < 0) ? 1 : cost;
    g->processes[id].priority      = (priority < 0) ? 1 : priority;
    g->processes[id].is_deadlocked = 0;
    strncpy(g->processes[id].name, name, MAX_NAME - 1);
    g->processes[id].name[MAX_NAME - 1] = '\0';
    g->num_processes++;
    printf("  [+] Process P%d ('%s') added  [cost=%d, priority=%d]\n",
           id, name, cost, priority);
    return id;
}

/* ─────────────────────────────────────────────────────────────
   graph_add_resource: register a new resource
   Returns: resource id (0-based), or -1 on failure
   ───────────────────────────────────────────────────────────── */
int graph_add_resource(Graph *g, const char *name, int instances)
{
    if (g->num_resources >= MAX_RESOURCES) {
        printf("  [!] Error: Maximum resource limit (%d) reached.\n",
               MAX_RESOURCES);
        return -1;
    }
    if (instances <= 0) {
        printf("  [!] Error: Resource must have at least 1 instance.\n");
        return -1;
    }
    int id = g->num_resources;
    g->resources[id].id                  = id;
    g->resources[id].total_instances     = instances;
    g->resources[id].available_instances = instances;
    strncpy(g->resources[id].name, name, MAX_NAME - 1);
    g->resources[id].name[MAX_NAME - 1] = '\0';
    g->num_resources++;
    printf("  [+] Resource R%d ('%s') added  [instances=%d]\n",
           id, name, instances);
    return id;
}

/* ─────────────────────────────────────────────────────────────
   graph_display: pretty-print current state of processes/resources
   ───────────────────────────────────────────────────────────── */
void graph_display(const Graph *g)
{
    print_banner("CURRENT SYSTEM STATE");

    /* ── Processes ── */
    printf("  PROCESSES  (%d registered)\n", g->num_processes);
    printf("  +------+------------+---------------+----------+-----------+\n");
    printf("  | PID  | Name       | Recovery Cost | Priority | Deadlocked|\n");
    printf("  +------+------------+---------------+----------+-----------+\n");
    for (int i = 0; i < g->num_processes; i++) {
        printf("  | P%-3d | %-10s | %-13d | %-8d | %-9s |\n",
               g->processes[i].id,
               g->processes[i].name,
               g->processes[i].recovery_cost,
               g->processes[i].priority,
               g->processes[i].is_deadlocked ? "YES" : "No");
    }
    printf("  +------+------------+---------------+----------+-----------+\n\n");

    /* ── Resources ── */
    printf("  RESOURCES  (%d registered)\n", g->num_resources);
    printf("  +------+------------+---------+-----------+\n");
    printf("  | RID  | Name       | Total   | Available |\n");
    printf("  +------+------------+---------+-----------+\n");
    for (int i = 0; i < g->num_resources; i++) {
        printf("  | R%-3d | %-10s | %-7d | %-9d |\n",
               g->resources[i].id,
               g->resources[i].name,
               g->resources[i].total_instances,
               g->resources[i].available_instances);
    }
    printf("  +------+------------+---------+-----------+\n\n");

    /* ── Allocation matrix ── */
    if (g->num_processes > 0 && g->num_resources > 0) {
        print_matrix_nr("ALLOCATION MATRIX [alloc[P][R]]",
                        (int (*)[MAX_RESOURCES]) g->allocation,
                        g->num_processes, g->num_resources, g);

        print_matrix_nr("REQUEST MATRIX [request[P][R]]",
                        (int (*)[MAX_RESOURCES]) g->request,
                        g->num_processes, g->num_resources, g);
    }
}

/* ─────────────────────────────────────────────────────────────
   graph_reset_state: clear deadlock flags and WFG for a fresh run
   ───────────────────────────────────────────────────────────── */
void graph_reset_state(Graph *g)
{
    for (int i = 0; i < g->num_processes; i++)
        g->processes[i].is_deadlocked = 0;
    memset(g->wfg,       0, sizeof(g->wfg));
    memset(g->rec_graph, 0, sizeof(g->rec_graph));
    g->wfg_built     = 0;
    g->deadlock_found = 0;
}
