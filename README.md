<p align="center">
   <img src="https://img.shields.io/badge/Deadlock%20Detection-Recovery-CF6B53?style=for-the-badge&logo=github&logoColor=white&labelColor=111827" alt="Deadlock Detection" />
</p>

<p align="center">
   <strong>
      Algorithmic detection and cost-minimal recovery for deadlocks in resource-allocation systems.
   </strong><br/>
   CLI-driven C simulator with Graphviz export, demo inputs, and analysis reports.
</p>

<p align="center">
   <a href="https://github.com/c-varshith/deadlock_detection_recovery"><img src="https://img.shields.io/badge/GitHub-Source%20repo-111827?style=for-the-badge&logo=github&logoColor=white" alt="Source repo" /></a>
   <a href="#features"><img src="https://img.shields.io/badge/Features-DFS%20%7C%20FW%20%7C%20Dijkstra-0EA5E9?style=for-the-badge&labelColor=111827" alt="Features" /></a>
   <a href="./LICENSE"><img src="https://img.shields.io/badge/License-MIT-111827?style=for-the-badge" alt="MIT license" /></a>
</p>

<p align="center">
   <a href="#project-overview">Overview</a> &bull;
   <a href="#features">Features</a> &bull;
   <a href="#running-locally">Setup</a> &bull;
   <a href="#tech-stack">Tech</a>
</p>

> **Default mode:** Full pipeline demo (non-interactive). Use sample inputs in `sample_inputs/`.

| At a glance | Value |
|---|---|
| Language | C (C99) |
| Build | `make` (GCC) |
| Visualization | Graphviz `.dot` exports |
| Detections | DFS, Floyd–Warshall, Topo-check |
| Recovery | Dijkstra-based minimum-cost victim selection |

---

## Features

- ✅ DFS-based cycle detection with explicit cycle tracebacks
- ✅ Floyd–Warshall transitive closure verification for global cycle detection
- ✅ Topological-sort sanity checks and BFS dependency exploration
- ✅ Weighted Dijkstra recovery strategies (terminate/preempt/priorities)
- ✅ Graphviz `.dot` export (`outputs/`) and demo `sample_inputs/`

---

## Project Overview

This simulator models a resource-allocation environment, builds a Resource Allocation Graph (RAG), converts it to a Wait-For Graph (WFG), runs detection algorithms, and (if needed) performs minimum-cost recovery using Dijkstra on a recovery graph.

```
Input → RAG → WFG → DFS / Floyd–Warshall / Topo → (if deadlock) Dijkstra recovery → Safe state
```

---

## Project Structure

```
./
├── main.c
├── graph.h
├── graph.c
├── rag.c
├── wfg.c
├── dfs_deadlock.c
├── floyd_warshall.c
├── topo_sort.c
├── bfs.c
├── dijkstra_recovery.c
├── visualization.c
├── utils.c
├── Makefile
├── sample_inputs/
│   ├── tc1_full_pipeline.txt
│   ├── tc2_full_pipeline.txt
│   └── tc3_full_pipeline.txt
├── outputs/        # .dot files generated at runtime (rag.dot, wfg.dot)
└── docs/
```

---

## Prerequisites

| Tool | Minimum |
|---|---|
| GCC / clang | C99-compatible |
| Make | GNU Make |
| Graphviz (optional) | `dot` for rendering `.dot` → PNG |

Install on Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential make graphviz
```

---

## Running Locally

### 1. Clone the repo

```bash
git clone https://github.com/c-varshith/deadlock_detection_recovery.git
cd deadlock_detection_recovery
```

### 2. Build

```bash
make
```

If `make` completes successfully, the CLI executable is produced (e.g., `deadlock_sim`).

### 3. Run (interactive or via sample input)

```bash
./deadlock_sim            # interactive menu-driven simulator
# or run a pipeline from sample input
./deadlock_sim < sample_inputs/tc1_full_pipeline.txt
```

### 4. Render Graphviz output

```bash
dot -Tpng outputs/wfg.dot -o outputs/wfg.png
dot -Tpng outputs/rag.dot -o outputs/rag.png
```

---

## Examples

Run the full pipeline demo (test case 1):

```bash
make clean && make && ./deadlock_sim < sample_inputs/tc1_full_pipeline.txt
```

---

## Algorithms & Notes

- **DFS Detection** — O(V + E). Uses three-color DFS to trace cycles.
- **Floyd–Warshall** — O(V³). Computes reachability closure for verification.
- **Topological Check** — O(V + E). Kahn's algorithm to detect unschedulable sets.
- **BFS Traversal** — O(V + E). Shows blocking propagation from a source.
- **Dijkstra Recovery** — O(V²). Builds a weighted recovery graph to pick minimum-cost victim(s).

Refer to inline comments in source files for implementation details and complexity notes.

---

## Documentation

See the `docs/` folder for architecture diagrams, demo guide, and project report.

---

## Contributing

Contributions welcome. Please open an issue for major changes and submit PRs with tests or reproducible examples. Keep commits focused and ensure `make` still builds.

---

## License

This project is licensed under the [MIT License](./LICENSE).

Copyright (c) 2026 Varshith C

See [LICENSE](./LICENSE) for the full license text.