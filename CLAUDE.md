# Ink — Project Context for Claude Code

## What This Project Is

Ink is a C++20 path-generation engine for timing analysis (EDA). It has two
parallel path-generation implementations being benchmarked against a sequential
golden:

| Name      | Entry point              | Function              | Status        |
|-----------|--------------------------|----------------------|---------------|
| **ot**    | `report_rebuild`         | sequential Yen's     | golden baseline |
| **pathgen** | `report_multiq`        | `_spur_multiq`       | exact — TBB concurrent_vector level+window loop |
| **cpathgen** | `report_paths_mlq`   | `_spur_mlq`          | exact (TBB delta-stepping) |

## Key Source Locations

### Modifiable functions
- `ink/ink.cpp:1865` — `Ink::_spur_multiq(size_t K, Pfxt&, bool, bool)` — top-level worker loop (TBB level+window, window drain)

### Read-only reference
- `ink/ink.cpp:1695` — `Ink::_spur_mlq` — cpathgen reference (level-by-level with delta-stepping)
- `ink/ink.cpp:2821` — `Ink::_spur_tbb_task_vecs` — per-node spur pushing children to task_vecs
- `ink/ink.cpp:1408` — `Ink::_pfxt_cache_multiq` — seeds pfxt.task_qs with src prefixes
- `ink/ink.cpp:3323` — `push_task` / `pop_task` — multi-queue task push/pop
- `ink/ink.hpp:510`  — `determine_q_idx` — maps path cost → queue index
- `ink/ink.hpp:530`  — `set_num_task_qs` — initialises queue bounds

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
  vga_lcd.edges       ← primary performance benchmark (largest graph)

golden/              ← true top-K path costs (k=20000) from gen-golden
  tv80.golden
  aes_core.golden
  des_perf.golden
  vga_lcd.golden

big-table.csv        ← benchmark results (written by big-table executable)
```

**Primary verification targets: leon2, leon3mp, netcard with K=1M** — these are
the largest graphs in the suite (3–4M vertices, vs vga_lcd's ~1M) and the most
sensitive to performance regressions. Use them for all optimization iterations.
If all three are correct (0% error) and all three show a speedup, that is a win.
Only run the full 7-benchmark suite (tv80, aes_core, des_perf, vga_lcd, leon2, leon3mp, netcard)
for final confirmation.

```bash
# Quick performance check (primary — all three large benchmarks)
rm -f big-table.csv
for bm in leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
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
  examples/cpathgen/gen-golden 20000 benchmarks/${bm}.edges golden/${bm}.golden
done

# Run big-table comparison
rm -f big-table.csv
for bm in tv80 aes_core des_perf vga_lcd; do
  examples/cpathgen/big-table 20000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

Note: the verify binary is `examples/cpathgen/big-table` (not `build/bin/big-table`).

## Current Implementation: `_spur_multiq` (TBB level+window)

The original Taskflow+moody-camel implementation was replaced with a
cpathgen-style TBB concurrent_vector level+window loop. This eliminates all
dequeue overhead and processes nodes in strict cost order, giving zero orphans.

**Algorithm:**
1. Seed `task_vecs[0..num_task_qs-1]` from `pfxt.task_qs` (filled by `_pfxt_cache_multiq`).
2. Process vecs 0..N-2 (active cost bands) with window loop via
   `tbb::parallel_for_each`; each node spurts via `_spur_tbb_task_vecs`
   (children routed to correct vec by cost); node itself pushed to `paths_cv`.
3. When all active vecs drain, promote overflow vec: bump bounds by `_width`,
   redistribute overflow nodes into active vecs, compact overflow.
4. Stop when `path_cnt >= K` or all vecs empty.
5. Window drain: compute tight threshold (K-th cost across result ∪ orphans);
   spur qualifying orphans in parallel via `_spur_tbb_task_vecs`; pull children
   back into drain_cv if ≤ threshold; repeat until drain_cv stops growing.
6. Merge drain result into top-K and enqueue to `pfxt.paths_concurr`.

**Result:** 0% error, drain_count=0 on all benchmarks (tv80, aes_core, des_perf,
vga_lcd) at both K=20K and K=1M. Pathgen matches ot speed at K=1M (165ms vs 157ms).

**Correctness invariant:** child cost ≥ parent cost (monotonicity). Any node with
cost > threshold has no descendant in the true top-K. The drain terminates exactly.

## Multi-Agent Debugging / Optimization Framework

`scripts/debug_agent.py` — uses **Gemini** via OpenAI-compatible API. Requires:
```bash
export GEMINI_API_KEY=<your key>
pip install openai  # the script uses openai SDK pointed at Gemini's endpoint
```

Provider is set at the top of the script: `PROVIDER = "gemini"` (already configured).
To switch back to Anthropic, change to `PROVIDER = "anthropic"` and set `ANTHROPIC_API_KEY`.

| Agent    | Model               | Input                               | Output                         |
|----------|---------------------|-------------------------------------|--------------------------------|
| Explorer | gemini-flash (fast) | source code                         | `debug_session/01_explorer.md` |
| Analyzer | gemini-flash (fast) | source code + big-table.csv         | `debug_session/02_analyzer.md` |
| Planner  | gemini-flash (fast) | explorer + analyzer reports + source | `debug_session/03_plan.md`    |

Explorer and Analyzer run in parallel. Planner runs after both finish.
**Claude acts as Critic, Coder, and Fixer** — reads `03_plan.md`, evaluates it,
implements the best direction, builds, and verifies.

When Claude has open questions it cannot resolve from context, it records them
in the `## Open Questions` section of CLAUDE.md and waits. The user answers by
highlighting the question and commenting in the IDE.

Run: `python scripts/debug_agent.py`

---

## Agentic Code Optimizer — Rules of Engagement

This section governs how Claude (as coordinator) runs an automated optimization
loop on `_spur_multiq` and related code. Follow all rules below precisely.

### Workflow

```
Claude develops optimization
  → build (cmake --build build --parallel)
  → verify correctness: examples/cpathgen/big-table 1000000 benchmarks/vga_lcd.edges golden/vga_lcd.golden
      pathgen_avg_err and pathgen_max_err must be 0.0
  → check performance vs baseline (pathgen_pfxt_time < 165ms = improvement)
  → if incorrect or hangs: fix or revert (see limits below)
  → when 3 consecutive optimizations fail to improve:
      rm -f debug_session/*.md   # clear cache to force fresh analysis
      python scripts/debug_agent.py
      read debug_session/03_plan.md
      Claude evaluates proposals, picks best direction, implements it
  → if Claude has questions it cannot answer: record in ## Open Questions in CLAUDE.md
      (user answers by highlighting + commenting in IDE)
  → repeat until no further improvement possible
```

### 1. Build and Test Regularly

After every non-trivial code change:
```bash
cmake --build build --parallel 2>&1 | tail -10
```
Then verify:
```bash
rm -f big-table.csv
for bm in tv80 aes_core des_perf vga_lcd; do
  examples/cpathgen/big-table 20000 benchmarks/${bm}.edges golden/${bm}.golden 2>&1
done
cat big-table.csv
```
**Correctness first**: pathgen_avg_err and pathgen_max_err must both be 0.0 before
checking performance. Never commit a "faster but wrong" result.

### 2. No-Progress Rule → Re-plan via Agent Group

If 3 consecutive optimization attempts fail to improve performance (or make it
worse), **stop and call the agent group**:
```bash
export GEMINI_API_KEY=<your key>   # required; pip install openai
rm -f debug_session/*.md           # clear cache so agents re-analyse fresh code
python scripts/debug_agent.py
```
- Explorer analyses current code structure and hot paths
- Analyzer reads current `big-table.csv` + source → produces bottleneck report
- Planner produces 3 ranked optimization proposals with effort/risk estimates
- Claude reads `debug_session/03_plan.md`, critiques each proposal, picks the
  best direction, and implements it (Claude is the Critic + Coder + Fixer)
- If Claude has open questions, they go in `## Open Questions` in CLAUDE.md

### 3. Build-Failure Limit and Revert Protocol

- **Max 3 fix attempts** on a failing build before reverting
- **Max 3 fix attempts** on a correctness failure before reverting
- To revert: `git stash` (or `git checkout ink/ink.cpp`) back to last clean commit
- After revert, record what was tried and why it failed before attempting anything new
- Tag clean working checkpoints with git: `git commit -m "checkpoint: <description>"`

### 4. Hang / Timeout Detection

The benchmark should complete in **< 5 seconds per benchmark** (K=20000).
If it runs longer:
1. Kill with Ctrl-C after 5s
2. Add `std::cerr` debug prints to identify which phase is hanging
   (main loop entry, per-vec processing, overflow promotion, drain)
3. Common parallel pitfalls to check:
   - **Infinite loop in overflow promotion**: `while (num_promoted == 0)` spins
     forever if `task_vecs.back()` is empty but you forgot the empty-check early exit
   - **Deadlock**: two threads each holding a lock the other needs
   - **Race on `bounds`**: bounds updated by one thread while another reads mid-update
   - **Race on `task_vecs`**: pushing to a vec while iterating it
4. Max 3 fix attempts, then revert

### 5. Performance Baseline (K=1M, vga_lcd)

| Metric              | Baseline value |
|---------------------|----------------|
| ot_pfxt_time        | ~157 ms        |
| pathgen_pfxt_time   | ~165 ms        |
| cpathgen_pfxt_time  | ~327 ms        |
| drain_count         | 0              |
| pathgen error       | 0.0%           |

An optimization "improves" if it reduces `pathgen_pfxt_time` by ≥ 5% without
increasing error. Always verify on vga_lcd K=1M first; run the full 4-benchmark
suite (K=20000) only for final confirmation before committing.

### 6. Areas to Explore (seed list for Planner)

- Reduce `_spur_tbb_task_vecs` per-node work (e.g., SIMD-friendly edge traversal,
  early cost pruning before allocation)
- Reduce `std::make_unique<PfxtNode>` allocation pressure (memory pool / slab)
- Reduce `tbb::concurrent_vector::push_back` contention (partitioner tuning,
  local buffers per thread then bulk-push)
- Overflow compaction: `remove_if` scans the whole overflow vec — consider a
  generation counter or two-vector swap instead
- Profile with `perf` or Instruments to find the actual hotspot before coding

### 7. Context Window Management

The optimizer loop can run many iterations and consume a lot of context. To keep
the context clean:
- When re-planning, delegate analysis to `scripts/debug_agent.py` rather than
  re-reading the entire source inline
- Summarize completed iterations (what was tried, result) in a short bullet list
  rather than reproducing full diffs
- The Critic agent in `debug_agent.py` can ask Claude questions interactively —
  use this for deep design decisions rather than thinking alone

## Open Questions

*(Claude records unresolved questions here during autonomous optimization runs.
User answers by highlighting the question and commenting in the IDE.)*

---

## Notes

- vga_lcd golden contains negative-cost paths (valid: both pathgen and cpathgen handle them)
- `big-table.cpp` skips zero-cost golden entries in error calculation (avoids nan)
- Platform: macOS (Apple Clang, Homebrew TBB + libomp); also builds on Linux
- `_spur_multiq` no longer uses Taskflow or moody camel queues — pure TBB throughout
