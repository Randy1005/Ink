# Per-Thread task_vecs + Thread Count Cap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Eliminate `tbb::concurrent_vector` push_back contention in `_spur_multiq` by giving each thread its own local `task_vecs`, flushing into the global vector after each window, and cap the default thread count at 4.

**Architecture:** `tbb::combinable<vector<concurrent_vector<...>>> tl_task_vecs` is declared at per-band scope. Inside `parallel_for_each`, each thread passes its own `tl_task_vecs.local()` to `_spur_tbb_task_vecs` instead of the shared global. After each window, `combine_each` bulk-flushes per-thread children into the global `task_vecs` via `grow_by`. The thread cap is applied inside the existing `if (_num_workers == 0)` guard.

**Tech Stack:** C++20, TBB (`tbb::combinable`, `tbb::concurrent_vector`, `oneapi::tbb::parallel_for_each`), CMake, `examples/cpathgen/big-table` for verification.

---

## File Map

| File | Change |
|------|--------|
| `ink/ink.cpp:1872–1874` | Thread count cap inside existing `if (_num_workers == 0)` guard |
| `ink/ink.cpp:1915` | Add `tl_task_vecs` combinable declaration alongside `local_paths` |
| `ink/ink.cpp:1924` | Change call site from `task_vecs` to `tl_task_vecs.local()` |
| `ink/ink.cpp:1928–1934` | Add `tl_task_vecs.combine_each` flush after existing `local_paths` combine_each |

No other files are modified. `_spur_tbb_task_vecs` and all other functions are untouched.

---

## Task 1: Confirm baseline is tagged and record reference numbers

**Files:** none (data collection only)

- [ ] **Step 1: Verify iter3-baseline tag exists**

```bash
git tag | grep iter3
```

Expected: `iter3-baseline` in output. If missing:
```bash
git tag iter3-baseline
```

- [ ] **Step 2: Run correctness + performance baseline at default thread count**

```bash
rm -f big-table.csv
for bm in vga_lcd leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

Expected: `pathgen_avg_err=0.0`, `pathgen_max_err=0.0` on all four rows. Record `pathgen_avg_pfxt_time` for leon2/leon3mp/netcard.

- [ ] **Step 3: Run baseline scalability sweep (iter3 numbers for comparison)**

```bash
rm -f big-table.csv
for T in 2 4 6 8 10; do
  examples/cpathgen/big-table 1000000 benchmarks/leon2.edges golden/leon2.golden $T
done
cat big-table.csv
```

Record all `pathgen_avg_pfxt_time` values per thread count. These are the iter3 scaling numbers to beat.

---

## Task 2: Apply thread count cap

**Files:**
- Modify: `ink/ink.cpp:1872–1874`

- [ ] **Step 1: Read the current guard**

Open `ink/ink.cpp` and locate lines 1872–1874:
```cpp
if (_num_workers == 0) {
  _num_workers = std::thread::hardware_concurrency();
}
```

- [ ] **Step 2: Apply the cap**

Change to:
```cpp
if (_num_workers == 0) {
  _num_workers = std::min(
    (size_t)std::thread::hardware_concurrency(),
    (size_t)4
  );
}
```

- [ ] **Step 3: Build**

```bash
cmake --build build --parallel 2>&1 | tail -5
```

Expected: zero errors, zero warnings on the changed lines.

- [ ] **Step 4: Verify correctness still holds**

```bash
rm -f big-table.csv
for bm in vga_lcd leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

Expected: `pathgen_avg_err=0.0`, `pathgen_max_err=0.0`. The default run now uses 4 threads — `pathgen_avg_pfxt_time` should be ~241/198/214ms for leon2/leon3mp/netcard (matching the iter3 4T numbers from the scaling study).

- [ ] **Step 5: Commit and tag**

```bash
git add ink/ink.cpp
git commit -m "perf: cap default _num_workers at 4 (sweet spot on M-series)"
git tag iter5-thread-cap
```

---

## Task 3: Add per-thread task_vecs buffering (core change)

**Files:**
- Modify: `ink/ink.cpp:1915, 1924, 1928–1934`

- [ ] **Step 1: Read the window processing block**

Open `ink/ink.cpp` and locate the per-band scope block starting at line 1909. Identify the three locations to change:
1. Line 1915: `tbb::combinable<std::vector<std::unique_ptr<PfxtNode>>> local_paths;` — add `tl_task_vecs` declaration immediately after this line.
2. Line 1924: `_spur_tbb_task_vecs(pfxt, *pfx, task_vecs);` — change `task_vecs` to `tl_task_vecs.local()`.
3. Lines 1928–1934: the existing `local_paths.combine_each` block — add a second `combine_each` for `tl_task_vecs` immediately after it, still inside `arena.execute([&]{...})`.

- [ ] **Step 2: Add `tl_task_vecs` declaration at per-band scope**

After line 1915 (the `local_paths` declaration), add:
```cpp
// Per-thread child buffers: eliminates push_back contention on shared task_vecs.
// Each thread writes children to its own local copy; combine_each bulk-flushes
// into global task_vecs after each window using grow_by.
tbb::combinable<std::vector<tbb::concurrent_vector<std::unique_ptr<PfxtNode>>>> tl_task_vecs(
  [&]{ return std::vector<tbb::concurrent_vector<std::unique_ptr<PfxtNode>>>(num_task_qs); }
);
```

Position: immediately after line 1915, before `while (wbeg < wend) {`.

- [ ] **Step 3: Change the call site inside the lambda**

On line 1924, change:
```cpp
_spur_tbb_task_vecs(pfxt, *pfx, task_vecs);
```
to:
```cpp
_spur_tbb_task_vecs(pfxt, *pfx, tl_task_vecs.local());
```

- [ ] **Step 4: Add the flush in combine_each**

The existing `local_paths.combine_each` block is at lines 1928–1934, inside `arena.execute([&]{...})`. After its closing `});`, add the `tl_task_vecs` flush (still inside `arena.execute([&]{...})`):

```cpp
// Flush per-thread child buffers into global task_vecs.
// clear() is called AFTER std::move, so all unique_ptrs are already null/moved-from
// before clear() runs — safe. tbb::concurrent_vector::clear() does not release
// segment memory, but per-thread footprint is bounded (num_task_qs bands) and
// stable after the first few windows — the retained segments are reused next window.
tl_task_vecs.combine_each([&](auto& local_tv) {
  for (size_t qi = 0; qi < num_task_qs; qi++) {
    if (!local_tv[qi].empty()) {
      auto it = task_vecs[qi].grow_by(local_tv[qi].size());
      std::move(local_tv[qi].begin(), local_tv[qi].end(), it);
      local_tv[qi].clear();
    }
  }
});
```

The full `arena.execute` block should now look like:
```cpp
arena.execute([&] {
  oneapi::tbb::parallel_for_each(
    task_vecs[id].begin() + wbeg,
    task_vecs[id].begin() + wend,
    [&](auto& pfx) {
      if (!pfx) return;
      _spur_tbb_task_vecs(pfxt, *pfx, tl_task_vecs.local());  // <-- changed
      local_paths.local().push_back(std::move(pfx));
    });
  // Bulk-insert into paths_cv and clear local buffers for reuse next window.
  local_paths.combine_each([&](std::vector<std::unique_ptr<PfxtNode>>& v) {
    if (!v.empty()) {
      auto it = paths_cv.grow_by(v.size());
      std::move(v.begin(), v.end(), it);
      v.clear();
    }
  });
  // Flush per-thread child buffers into global task_vecs.
  tl_task_vecs.combine_each([&](auto& local_tv) {
    for (size_t qi = 0; qi < num_task_qs; qi++) {
      if (!local_tv[qi].empty()) {
        auto it = task_vecs[qi].grow_by(local_tv[qi].size());
        std::move(local_tv[qi].begin(), local_tv[qi].end(), it);
        local_tv[qi].clear();
      }
    }
  });
});
```

- [ ] **Step 5: Build**

```bash
cmake --build build --parallel 2>&1 | tail -10
```

Expected: zero errors. If there is a type error, check that `tl_task_vecs` holds `std::vector<tbb::concurrent_vector<std::unique_ptr<PfxtNode>>>` — this must exactly match `_spur_tbb_task_vecs`'s parameter type.

If build fails 3 times, revert:
```bash
git checkout ink/ink.cpp
```

- [ ] **Step 6: Correctness check (K=1M, all large benchmarks)**

```bash
rm -f big-table.csv
for bm in vga_lcd leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

**Must pass:** `pathgen_avg_err=0.0` and `pathgen_max_err=0.0` on all four rows, `drain_count=0` (check stderr).

If error > 0.0: the flush is missing children. Check that `tl_task_vecs.combine_each` is inside `arena.execute` and that `local_tv[qi].clear()` is called after the move, not before.

If build or correctness fails 3 times, revert to baseline:
```bash
git checkout iter3-baseline -- ink/ink.cpp
cmake --build build --parallel
```

- [ ] **Step 7: Performance check (K=1M, 4T default)**

```bash
rm -f big-table.csv
for bm in leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

Compare `pathgen_avg_pfxt_time` against Task 1 Step 2 numbers (iter3, default thread count now = 4T after the cap). **Success:** ≥5% improvement on at least 2 of 3 benchmarks.

> **Note — 11T comparison:** The spec also requests an 11T comparison. Since Task 2 has already capped `_num_workers` at 4, running `big-table` without a thread argument will use 4T. To get an uncapped 11T run for comparison, check out the baseline temporarily:
> ```bash
> git stash
> git checkout iter3-baseline -- ink/ink.cpp
> cmake --build build --parallel
> rm -f big-table.csv
> for bm in leon2 leon3mp netcard; do
>   examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
> done
> cat big-table.csv   # iter3 11T numbers
> git checkout HEAD -- ink/ink.cpp && cmake --build build --parallel && git stash pop
> ```
> iter3 11T baselines: leon2 ~259ms, leon3mp ~212ms, netcard ~242ms.

- [ ] **Step 8: Commit and tag**

```bash
git add ink/ink.cpp
git commit -m "perf: per-thread task_vecs buffering eliminates concurrent_vector push_back contention"
git tag iter5-tl-task-vecs
```

---

## Task 4: Scalability sweep

**Files:** none (measurement only)

- [ ] **Step 1: Run scalability sweep with new code**

```bash
rm -f big-table-scaling.csv
for T in 2 4 6 8 10; do
  rm -f big-table.csv
  examples/cpathgen/big-table 1000000 benchmarks/leon2.edges golden/leon2.golden $T
  echo "=== T=$T ===" && cat big-table.csv
done
```

Note: `big-table` accepts an optional 4th argument `[num_threads]` which calls `pathgen.set_num_workers(T)` directly, bypassing the `_num_workers == 0` guard in `_spur_multiq`.

- [ ] **Step 2: Compare against iter3 scaling numbers**

Tabulate `pathgen_avg_pfxt_time` for each T alongside the iter3 numbers from Task 1 Step 3.

Expected pattern if contention was the bottleneck: higher thread counts (6T, 8T, 10T) should now show better speedups than iter3. The scaling curve should be flatter (less degradation at high T).

- [ ] **Step 3: Record results in experiment log**

Append to `debug_session/experiment_log.md`:

```markdown
## Iteration 5

**Optimizations:** per-thread task_vecs (tbb::combinable) + _num_workers cap at 4

### Outcome (K=1M, 5-run avg)

| Benchmark | iter3 4T (ms) | iter5 4T (ms) | Δ% |
|-----------|--------------|--------------|-----|
| leon2     | 241.1        | <fill>       | <fill> |
| leon3mp   | 197.7        | <fill>       | <fill> |
| netcard   | 214.1        | <fill>       | <fill> |

### Scalability (leon2, K=1M)

| threads | iter3 (ms) | iter5 (ms) | Δ% |
|---------|-----------|-----------|-----|
| 2       | 308.8     | <fill>    | <fill> |
| 4       | 241.1     | <fill>    | <fill> |
| 6       | 255.0     | <fill>    | <fill> |
| 8       | 258.3     | <fill>    | <fill> |
| 10      | 265.7     | <fill>    | <fill> |

drain_count: <fill> (must be 0)
pathgen_avg_err: <fill> (must be 0.0)
```

- [ ] **Step 4: Commit experiment log**

```bash
git add debug_session/experiment_log.md
git commit -m "results: iter5 per-thread task_vecs scalability measurements"
```

---

## Quick Reference: Revert to Baseline

```bash
# Revert ink.cpp only (keep everything else)
git checkout iter3-baseline -- ink/ink.cpp
cmake --build build --parallel

# Full revert (nuclear option)
git stash
git checkout iter3-baseline
cmake --build build --parallel
```
