/*
 * visualization.c — ASCII Visualization and Graphviz Export
 *
 * Provides:
 *   ascii_rag()     — text-art depiction of the RAG
 *   ascii_wfg()     — text-art depiction of the WFG
 *   export_graphviz() — write a .dot file renderable by Graphviz
 *
 * Graphviz rendering:
 *   dot -Tpng output.dot -o output.png
 *   dot -Tsvg output.dot -o output.svg
 */

#include "graph.h"

/* ─────────────────────────────────────────────────────────────
   ascii_rag: draw the RAG using text symbols
   ───────────────────────────────────────────────────────────── */
void ascii_rag(const Graph *g)
{
    print_banner("ASCII RESOURCE ALLOCATION GRAPH");

    if (g->num_processes == 0 && g->num_resources == 0) {
        printf("  (empty graph)\n\n"); return;
    }

    /* Processes legend */
    printf("  Processes  (○ = process node):\n    ");
    for (int p = 0; p < g->num_processes; p++)
        printf("  (P%d) ", p);
    printf("\n\n");

    /* Resources legend */
    printf("  Resources  (□ = resource node):\n    ");
    for (int r = 0; r < g->num_resources; r++)
        printf("  [R%d] ", r);
    printf("\n\n");

    /* Allocation edges: Rr ──▶ Pp */
    printf("  Allocation edges  R ──▶ P :\n");
    int any = 0;
    for (int r = 0; r < g->num_resources; r++) {
        for (int p = 0; p < g->num_processes; p++) {
            if (g->allocation[p][r] > 0) {
                printf("    [R%d] ─────(%d)─────▶ (P%d)\n",
                       r, g->allocation[p][r], p);
                any = 1;
            }
        }
    }
    if (!any) printf("    (none)\n");

    /* Request edges: Pp ──▶ Rr */
    printf("\n  Request edges  P ──▶ R :\n");
    any = 0;
    for (int p = 0; p < g->num_processes; p++) {
        for (int r = 0; r < g->num_resources; r++) {
            if (g->request[p][r] > 0) {
                printf("    (P%d) - - -(%d)- - -> [R%d]\n",
                       p, g->request[p][r], r);
                any = 1;
            }
        }
    }
    if (!any) printf("    (none)\n");
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
   ascii_wfg: draw the WFG as a matrix + arrow list
   ───────────────────────────────────────────────────────────── */
void ascii_wfg(const Graph *g)
{
    print_banner("ASCII WAIT-FOR GRAPH");

    if (!g->wfg_built) {
        printf("  [!] WFG not built. Run option 6 first.\n\n"); return;
    }

    int n = g->num_processes;
    if (n == 0) { printf("  (empty)\n\n"); return; }

    printf("  Waiting-dependency arrows:\n\n");
    int any = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (g->wfg[i][j]) {
                printf("    (P%d) ════════════▶ (P%d)   "
                       "[ %s waits for %s ]\n",
                       i, j,
                       g->processes[i].name,
                       g->processes[j].name);
                any = 1;
            }
        }
    }
    if (!any) printf("    (no wait-for dependencies)\n");
    printf("\n");

    /* Mark deadlocked processes */
    printf("  Process status:\n");
    for (int i = 0; i < n; i++) {
        printf("    P%d (%s) — %s\n",
               i, g->processes[i].name,
               g->processes[i].is_deadlocked ? "⚠ DEADLOCKED" : "✔ Safe");
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
   export_graphviz: write a Graphviz .dot file
   ───────────────────────────────────────────────────────────── */
void export_graphviz(const Graph *g, const char *filename)
{
    print_banner("GRAPHVIZ EXPORT");

    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("  [!] Could not open '%s' for writing.\n\n", filename);
        return;
    }

    fprintf(f, "// Deadlock Simulation — Resource Allocation Graph\n");
    fprintf(f, "// Render: dot -Tpng %s -o rag.png\n\n", filename);
    fprintf(f, "digraph RAG {\n");
    fprintf(f, "    graph [rankdir=LR, fontname=\"Helvetica\", "
                         "label=\"Resource Allocation Graph\", "
                         "fontsize=14];\n");
    fprintf(f, "    node  [fontname=\"Helvetica\", fontsize=11];\n\n");

    /* ── Process nodes (ellipse) ──────────────────────────── */
    fprintf(f, "    // Processes\n");
    for (int p = 0; p < g->num_processes; p++) {
        const char *fill = g->processes[p].is_deadlocked
                           ? "\"#FF6B6B\"" : "\"#AED6F1\"";
        fprintf(f, "    P%d [label=\"P%d\\n(%s)\\ncost=%d\", "
                   "shape=ellipse, style=filled, fillcolor=%s];\n",
                p, p, g->processes[p].name,
                g->processes[p].recovery_cost, fill);
    }
    fprintf(f, "\n");

    /* ── Resource nodes (box) ─────────────────────────────── */
    fprintf(f, "    // Resources\n");
    for (int r = 0; r < g->num_resources; r++) {
        fprintf(f, "    R%d [label=\"R%d\\n(%s)\\n%d/%d\", "
                   "shape=box, style=filled, fillcolor=\"#A9DFBF\"];\n",
                r, r, g->resources[r].name,
                g->resources[r].available_instances,
                g->resources[r].total_instances);
    }
    fprintf(f, "\n");

    /* ── Allocation edges  R → P ──────────────────────────── */
    fprintf(f, "    // Allocation edges (R -> P)\n");
    for (int r = 0; r < g->num_resources; r++) {
        for (int p = 0; p < g->num_processes; p++) {
            if (g->allocation[p][r] > 0) {
                fprintf(f, "    R%d -> P%d [label=\"%d\", "
                           "color=\"#27AE60\", penwidth=2];\n",
                        r, p, g->allocation[p][r]);
            }
        }
    }
    fprintf(f, "\n");

    /* ── Request edges  P → R ─────────────────────────────── */
    fprintf(f, "    // Request edges (P -> R)\n");
    for (int p = 0; p < g->num_processes; p++) {
        for (int r = 0; r < g->num_resources; r++) {
            if (g->request[p][r] > 0) {
                fprintf(f, "    P%d -> R%d [label=\"%d\", "
                           "style=dashed, color=\"#E74C3C\", penwidth=2];\n",
                        p, r, g->request[p][r]);
            }
        }
    }
    fprintf(f, "\n");

    /* ── WFG overlay (if built) ───────────────────────────── */
    if (g->wfg_built) {
        fprintf(f, "    // Wait-For edges (WFG overlay)\n");
        for (int i = 0; i < g->num_processes; i++) {
            for (int j = 0; j < g->num_processes; j++) {
                if (g->wfg[i][j]) {
                    fprintf(f, "    P%d -> P%d [label=\"waits\", "
                               "style=bold, color=\"#8E44AD\", "
                               "constraint=false];\n", i, j);
                }
            }
        }
    }

    fprintf(f, "}\n");
    fclose(f);

    printf("  Graphviz file written to: %s\n\n", filename);
    printf("  To render:\n");
    printf("    dot  -Tpng %s -o rag.png   (PNG image)\n", filename);
    printf("    dot  -Tsvg %s -o rag.svg   (SVG vector)\n", filename);
    printf("    dot  -Tpdf %s -o rag.pdf   (PDF document)\n", filename);
    printf("    xdot %s                     (interactive viewer)\n\n",
           filename);

    /* Also write a WFG-only dot file */
    if (g->wfg_built) {
        char wfg_file[256];
        snprintf(wfg_file, sizeof(wfg_file), "outputs/wfg.dot");
        FILE *fw = fopen(wfg_file, "w");
        if (fw) {
            fprintf(fw, "digraph WFG {\n");
            fprintf(fw, "    graph [label=\"Wait-For Graph\", fontsize=14];\n");
            for (int i = 0; i < g->num_processes; i++) {
                const char *fill = g->processes[i].is_deadlocked
                                   ? "\"#FF6B6B\"" : "\"#AED6F1\"";
                fprintf(fw, "    P%d [label=\"P%d\\n%s\", "
                             "shape=ellipse, style=filled, fillcolor=%s];\n",
                        i, i, g->processes[i].name, fill);
            }
            for (int i = 0; i < g->num_processes; i++)
                for (int j = 0; j < g->num_processes; j++)
                    if (g->wfg[i][j])
                        fprintf(fw, "    P%d -> P%d;\n", i, j);
            fprintf(fw, "}\n");
            fclose(fw);
            printf("  WFG dot file written to: %s\n\n", wfg_file);
        }
    }
}
