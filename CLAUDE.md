# Ink — Project Context for Claude Code

## What This Project Is

Ink is a C++20 path-generation engine for timing analysis (EDA). It has two
parallel path-generation implementations being benchmarked against a sequential
golden:

| Name      | Entry point              | Function              | Status        |
|-----------|--------------------------|----------------------|---------------|
| **ot**    | `report_rebuild`         | sequential Yen's     | golden baseline |
| **pathgen** | `report_multiq`        | `_spur_multiq`       | has cost errors — BEING FIXED |
| **cpathgen** | `report_paths_mlq`   | `_spur_mlq`          | exact (TBB delta-stepping) |

## Key Source Locations

### The only functions to be modified (pathgen fix)
- `ink/ink.cpp:1863` — `Ink::_spur_multiq(size_t K, Pfxt&, bool, bool)` — top-level worker loop
- `ink/ink.cpp:2711` — `Ink::_spur_multiq(Pfxt&, PfxtNode&, tf::Executor&)` — per-node spur

**All other functions are read-only.**

### Read-only reference
- `ink/ink.cpp:1698` — `Ink::_spur_mlq` — exact cpathgen reference (level-by-level)
- `ink/ink.cpp:2752` — `Ink::_spur_tbb_task_vecs` — cpathgen per-node reference
- `ink/ink.cpp:1411` — `Ink::_pfxt_cache_multiq` — queue cache helper
- `ink/ink.cpp:3254` — `push_task` / `pop_task` — multi-queue task push/pop
- `ink/ink.hpp:524`  — `determine_q_idx` — maps path cost → queue index
- `ink/ink.hpp:544`  — `set_num_task_qs` — initialises queue bounds

### macOS compat layer (ink/ink.hpp, top of file)
Apple Clang lacks parallel STL. Two macros abstract this:
- `INK_EXECUTION_PAR_UNSEQ` — expands to `ink_exec::par_unseq` (Apple) or `std::execution::par_unseq`
- `INK_PAR_ALGO` — expands to `ink_compat` (Apple, sequential fallback) or `std`

## Benchmarks and Golden Files

```
benchmarks/          ← .edges input graphs
  tv80.edges
  aes_core.edges
  des_perf.edges
  vga_lcd.edges

golden/              ← true top-K path costs (k=20000) from gen-golden
  tv80.golden
  aes_core.golden
  des_perf.golden
  vga_lcd.golden

big-table.csv        ← benchmark results (written by big-table executable)
```

Run order: `gen-golden` → `big-table` → inspect csv.

## Build

```bash
# Configure (first time)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Rebuild after changes to ink.cpp / ink.hpp
cmake --build build --parallel

# Regenerate golden files (k=20000)
for bm in tv80 aes_core des_perf vga_lcd; do
  build/bin/gen-golden 20000 benchmarks/${bm}.edges golden/${bm}.golden
done

# Run big-table comparison
rm -f big-table.csv
for bm in tv80 aes_core des_perf vga_lcd; do
  build/bin/big-table 20000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

## The Bug and the Fix

### Root cause: early termination drops in-flight top-K paths

1. `_atom_path_cnt` reaches `K_overgrow` → outer `while` loop stops submitting tasks
2. `executor.wait_for_all()` waits for already-submitted tasks
3. Those tasks call the per-node `_spur_multiq` which pushes children into
   `pfxt.task_qs` via `push_task` — **children are never picked up**
4. Some orphaned children belong to the true top-K → missing from `paths_concurr`
   → path cost errors

`K_overgrow` is a heuristic, not a correctness mechanism. It must not be the
basis of the fix.

### Chosen fix: Threshold-Based Drain (Option D — provably exact)

**Algorithm (modify `_spur_multiq` top-level only):**
1. Run parallel phase until `_atom_path_cnt >= K` (stop using `K_overgrow`).
2. After `wait_for_all()`, sort `paths_concurr`; `threshold` = cost of K-th path.
3. Drain all `task_qs`: for each queued node with `cost <= threshold`, spur it
   sequentially and add result to `paths_concurr`; skip nodes with `cost > threshold`.
4. Re-sort `paths_concurr`; recompute `threshold` from new K-th path.
5. Repeat steps 3–4 until no queued node has `cost <= threshold`.
6. Return top-K from `paths_concurr`.

**Correctness proof:** After the drain loop, every remaining queued node has
`cost > threshold`. Since spur costs are monotonically non-decreasing along any
chain (child cost ≥ parent cost), no descendant of those nodes can have
`cost <= threshold`. Therefore `paths_concurr` contains the true top-K. QED.

## Multi-Agent Debugging Framework

`scripts/debug_agent.py` — Python + Anthropic API, 4 agents:

| Agent    | Model  | Input                                  | Output                    |
|----------|--------|----------------------------------------|---------------------------|
| Explorer | Sonnet | source code                            | `debug_session/01_explorer.md` |
| Analyzer | Sonnet | source code + big-table.csv            | `debug_session/02_analyzer.md` |
| Planner  | Opus   | explorer + analyzer reports + source   | `debug_session/03_plan.md` |
| Critic   | Opus   | all reports + source (interactive)     | `debug_session/04_critique.md` |

Explorer and Analyzer run in parallel (both work from source code directly).
Planner and Critic run sequentially after both finish.
Critic uses an `ask_user` tool to prompt the developer interactively in the terminal.

Run: `python scripts/debug_agent.py`

## Notes

- vga_lcd golden contains negative-cost paths (valid: cpathgen generates same values)
- `big-table.cpp` skips zero-cost golden entries in error calculation (avoids nan)
- Platform: macOS (Apple Clang, Homebrew TBB + libomp); also builds on Linux
