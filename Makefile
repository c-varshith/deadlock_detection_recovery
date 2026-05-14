# ═══════════════════════════════════════════════════════════════
# Makefile — Deadlock Simulator (DAA Project)
# Build: make
# Run:   ./deadlock_sim
# Clean: make clean
# ═══════════════════════════════════════════════════════════════

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g -O2
TARGET  = deadlock_sim
SRCDIR  = .

SRCS    = main.c          \
          graph.c         \
          rag.c           \
          wfg.c           \
          dfs_deadlock.c  \
          floyd_warshall.c\
          topo_sort.c     \
          bfs.c           \
          dijkstra_recovery.c \
          visualization.c \
          utils.c

OBJS    = $(SRCS:.c=.o)
HEADERS = graph.h

# ─── Default target ────────────────────────────────────────────
all: dirs $(TARGET)
	@echo ""
	@echo "  Build successful!  Run with:  ./$(TARGET)"
	@echo ""

# ─── Create output directories ─────────────────────────────────
dirs:
	@mkdir -p outputs sample_inputs docs

# ─── Link ──────────────────────────────────────────────────────
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "  Linked: $@"

# ─── Compile each .c ───────────────────────────────────────────
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo "  Compiled: $<"

# ─── Clean ─────────────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "  Cleaned build artifacts."

# ─── Deep clean (includes outputs) ─────────────────────────────
distclean: clean
	rm -rf outputs/*.dot outputs/*.png
	@echo "  Deep clean complete."

# ─── Run a quick demo (Test Case 1, full pipeline) ─────────────
demo: $(TARGET)
	@echo "16\n1\n18\n0" | ./$(TARGET)

# ─── Graphviz render (if dot is installed) ─────────────────────
render:
	@if command -v dot >/dev/null 2>&1; then \
	    dot -Tpng outputs/rag.dot -o outputs/rag.png && \
	    echo "  Rendered: outputs/rag.png"; \
	    dot -Tpng outputs/wfg.dot -o outputs/wfg.png && \
	    echo "  Rendered: outputs/wfg.png"; \
	else \
	    echo "  [!] Graphviz not found. Install with: sudo apt install graphviz"; \
	fi

.PHONY: all clean distclean dirs demo render
