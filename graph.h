/*
 * graph.h — Shared types and declarations
 * Project: Algorithmic Detection and Resolution of Deadlocks in RAGs
 * Language: C (C99, GCC compatible)
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════════
   CONSTANTS
   ═══════════════════════════════════════════════════════════════ */
#define MAX_PROCESSES  20
#define MAX_RESOURCES  20
#define MAX_NAME       16
#define INF            999999

/* ═══════════════════════════════════════════════════════════════
   DATA STRUCTURES
   ═══════════════════════════════════════════════════════════════ */

typedef struct {
    int  id;
    char name[MAX_NAME];
    int  recovery_cost;   /* cost to terminate/preempt this process     */
    int  priority;        /* higher value = more critical (kept longer) */
    int  is_deadlocked;   /* flag set after detection                   */
} Process;

typedef struct {
    int  id;
    char name[MAX_NAME];
    int  total_instances;
    int  available_instances;
} Resource;

typedef struct {
    /* ── Entities ──────────────────────────────────────────── */
    Process  processes[MAX_PROCESSES];
    Resource resources[MAX_RESOURCES];
    int      num_processes;
    int      num_resources;

    /* ── RAG matrices ──────────────────────────────────────── */
    int allocation[MAX_PROCESSES][MAX_RESOURCES]; /* alloc[p][r] = instances held    */
    int request   [MAX_PROCESSES][MAX_RESOURCES]; /* request[p][r] = instances wanted*/

    /* ── Wait-For Graph (process × process) ───────────────── */
    int wfg[MAX_PROCESSES][MAX_PROCESSES];        /* wfg[i][j]=1 → Pi waits for Pj  */

    /* ── Weighted recovery graph ───────────────────────────── */
    int rec_graph[MAX_PROCESSES][MAX_PROCESSES];  /* edge cost for Dijkstra recovery */

    /* ── State flags ───────────────────────────────────────── */
    int wfg_built;        /* 1 if WFG has been constructed */
    int deadlock_found;   /* 1 if any detection method found a cycle */
} Graph;

/* ═══════════════════════════════════════════════════════════════
   FUNCTION DECLARATIONS
   ═══════════════════════════════════════════════════════════════ */

/* ── graph.c ──────────────────────────────────────────────── */
void graph_init         (Graph *g);
int  graph_add_process  (Graph *g, const char *name, int cost, int priority);
int  graph_add_resource (Graph *g, const char *name, int instances);
void graph_display      (const Graph *g);
void graph_reset_state  (Graph *g);

/* ── rag.c ────────────────────────────────────────────────── */
int  rag_allocate       (Graph *g, int pid, int rid, int instances);
int  rag_request        (Graph *g, int pid, int rid, int instances);
void rag_display        (const Graph *g);
void rag_random         (Graph *g, int np, int nr);
void rag_test_case      (Graph *g, int tc);

/* ── wfg.c ────────────────────────────────────────────────── */
void wfg_build          (Graph *g);
void wfg_display        (const Graph *g);
void wfg_build_rec_graph(Graph *g);

/* ── dfs_deadlock.c ──────────────────────────────────────── */
int  dfs_detect_deadlock(Graph *g);

/* ── floyd_warshall.c ────────────────────────────────────── */
void floyd_warshall     (const Graph *g);

/* ── topo_sort.c ─────────────────────────────────────────── */
void topo_sort_check    (const Graph *g);

/* ── bfs.c ───────────────────────────────────────────────── */
void bfs_traversal      (const Graph *g, int start_pid);
void bfs_all            (const Graph *g);

/* ── dijkstra_recovery.c ─────────────────────────────────── */
void dijkstra_recovery  (Graph *g);

/* ── visualization.c ─────────────────────────────────────── */
void ascii_rag          (const Graph *g);
void ascii_wfg          (const Graph *g);
void export_graphviz    (const Graph *g, const char *filename);

/* ── utils.c ─────────────────────────────────────────────── */
void print_separator    (void);
void print_banner       (const char *title);
long get_time_us        (void);
void performance_report (Graph *g);
void print_matrix_np    (const char *label,
                         int mat[][MAX_PROCESSES], int n);
void print_matrix_nr    (const char *label,
                         int mat[][MAX_RESOURCES],
                         int rows, int cols,
                         const Graph *g);

#endif /* GRAPH_H */
