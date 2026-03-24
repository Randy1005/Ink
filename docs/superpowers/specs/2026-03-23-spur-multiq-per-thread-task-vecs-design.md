# Design: Per-Thread task_vecs + Thread Count Cap for `_spur_multiq`

**Date:** 2026-03-23
**Status:** Approved
**Target function:** `Ink::_spur_multiq` (`ink/ink.cpp:1865`)

---

## Problem

At K=1M on large benchmarks (leon2/leon3mp/netcard), `pathgen` (`_spur_multiq`) runs ~1.7× slower than `cpathgen` (`_spur_mlq`) despite both calling the same `_spur_tbb_task_vecs` kernel. Both have a sequential outer band loop with parallel inner processing. The structural overhead in `_spur_multiq` is dominated by **`tbb::concurrent_vector::push_back` contention**: during each window's `parallel_for_each`, every thread calls `_spur_tbb_task_vecs` which directly pushes children into the shared global `task_vecs[vec_id]`. When multiple threads route children to the same cost band, they contend on that band's internal atomics.

Additionally, the thread scaling study (experiment_log.md) showed that `hardware_concurrency()` = 11 threads is suboptimal on this machine (Apple M-series, memory-bandwidth-bound). 4 threads yields +7–12% for pathgen and +19–24% for cpathgen.

---

## Approach

### A: Per-Thread task_vecs (core change)

Replace the shared `task_vecs` passed to `_spur_tbb_task_vecs` with per-thread local copies via `tbb::combinable`. Each thread writes children into its own `tl_task_vecs.local()` (zero contention). After each window's `parallel_for_each`, a `combine_each` bulk-flushes per-thread children into the global `task_vecs` using `grow_by` (one atomic per non-empty band per thread).

`_spur_tbb_task_vecs` is read-only — its signature takes `std::vector<tbb::concurrent_vector<std::unique_ptr<PfxtNode>>>&`. The local combinable must hold the same nested type to match the signature. Each per-thread `tbb::concurrent_vector` still uses atomic operations internally (single-threaded), but single-threaded atomics are cheap — the dominant cost being eliminated is **cache-line thrashing** between threads contending on the same shared band. Uncontended atomics are an order of magnitude cheaper than heavily contested ones. The local bands are cleared via `clear()` after each flush; `tbb::concurrent_vector::clear()` does not release segment memory, but since the number of bands (`num_task_qs`) and segments are bounded, the TLS footprint per thread is bounded and stable after the first few windows.

### B: Thread Count Cap

Cap `_num_workers` at 4, but only when it would otherwise be set from `hardware_concurrency()` — do not override a value already explicitly set by a caller (e.g., via `set_num_workers()`):
```cpp
if (_num_workers == 0) {
  _num_workers = std::min((size_t)std::thread::hardware_concurrency(), (size_t)4);
}
```
This preserves the existing semantics of the `if (_num_workers == 0)` guard while capping the default.

---

## Data Flow (before / after)

**Before:**
```
parallel_for_each(window):
  thread A → _spur_tbb_task_vecs(..., global_task_vecs)  → push_back band 3  ↘
  thread B → _spur_tbb_task_vecs(..., global_task_vecs)  → push_back band 3  ↗  CONTENTION
  thread C → _spur_tbb_task_vecs(..., global_task_vecs)  → push_back band 3  ↗
```

**After:**
```
parallel_for_each(window):
  thread A → _spur_tbb_task_vecs(..., tl_task_vecs[A])   → push_back local band 3  (no contention)
  thread B → _spur_tbb_task_vecs(..., tl_task_vecs[B])   → push_back local band 3  (no contention)
  thread C → _spur_tbb_task_vecs(..., tl_task_vecs[C])   → push_back local band 3  (no contention)

combine_each (after window):
  tl_task_vecs[A..C] → grow_by → global_task_vecs[qi]   (1 atomic per non-empty band per thread)
```

---

## Implementation Details

### Scope of changes
Only `Ink::_spur_multiq` (`ink/ink.cpp:1865`). `_spur_tbb_task_vecs` and all other functions are untouched.

### Changes within `_spur_multiq`

1. **Thread count cap** inside the existing `if (_num_workers == 0)` guard (preserves caller-set values):
   ```cpp
   if (_num_workers == 0) {
     _num_workers = std::min((size_t)std::thread::hardware_concurrency(), (size_t)4);
   }
   ```

2. **Declare `tl_task_vecs` combinable** at per-band scope (alongside the existing `local_paths` combinable, outside the `while (wbeg < wend)` loop):
   ```cpp
   tbb::combinable<std::vector<tbb::concurrent_vector<std::unique_ptr<PfxtNode>>>> tl_task_vecs(
     [&]{ return std::vector<tbb::concurrent_vector<std::unique_ptr<PfxtNode>>>(num_task_qs); }
   );
   ```

3. **Change the call site** inside `parallel_for_each` lambda:
   ```cpp
   // Before:
   _spur_tbb_task_vecs(pfxt, *pfx, task_vecs);
   // After:
   _spur_tbb_task_vecs(pfxt, *pfx, tl_task_vecs.local());
   ```

4. **Add flush in `combine_each`** (after the existing `local_paths` flush):
   ```cpp
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

### Lifecycle / reuse
`tl_task_vecs` is hoisted to per-band scope so TLS slot allocation happens once per band (not per window). The `clear()` in `combine_each` resets each thread's local bands for reuse in the next window iteration.

---

## Correctness

**Cost monotonicity invariant:** A child node's cost ≥ its parent's cost. Nodes in band `id` therefore generate children destined for bands `>= id` (or overflow). No child can land in a band `< id` that has already been cleared. This means:

- During processing of band `id`, the global `task_vecs[id]` is only extended (beyond `wend`) by the `combine_each` after each window — never by concurrent push_back mid-iteration.
- Children landing in band `id` (same-band) appear beyond `wend` after the flush and are picked up by `wend = task_vecs[id].size()` in the next window iteration — identical to existing behavior.

**Drain phase:** The drain phase (lines ~2040–2120) also calls `_spur_tbb_task_vecs(pfxt, *pfx, task_vecs)` directly against the global `task_vecs`. This is intentionally left unchanged — `drain_count = 0` on all current benchmarks, so the drain phase contributes negligible runtime. If drain_count becomes non-zero in future workloads, the same per-thread buffering pattern can be applied there independently.

**No change to:** overflow promotion logic, drain_cv, or result finalization.

---

## Verification Plan

1. Build: `cmake --build build --parallel`
2. Correctness (K=20K, all 4 benchmarks):
   ```bash
   rm -f big-table.csv
   for bm in tv80 aes_core des_perf vga_lcd; do
     examples/cpathgen/big-table 20000 benchmarks/${bm}.edges golden/${bm}.golden
   done
   cat big-table.csv
   ```
   `pathgen_avg_err` and `pathgen_max_err` must both be `0.0`.

3. Performance (K=1M, primary benchmarks) — run at both 11T and 4T to compare against both baselines:
   ```bash
   # 11-thread run (default hardware_concurrency before cap)
   rm -f big-table.csv
   for bm in leon2 leon3mp netcard; do
     examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
   done
   cat big-table.csv

   # 4-thread run (cap applied)
   rm -f big-table.csv
   for bm in leon2 leon3mp netcard; do
     examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
   done
   cat big-table.csv
   ```
   Compare `pathgen_pfxt_time` vs:
   - 11T baseline: 259ms / 212ms / 242ms (leon2 / leon3mp / netcard)
   - 4T baseline:  241ms / 198ms / 214ms (from thread-scaling study, iter 3 code)

4. **Scalability test** (K=1M, leon2 only, 5-run average per thread count) — determine whether per-thread buffering improves scaling beyond 4T:
   ```bash
   for T in 2 4 6 8 10; do
     # set _num_workers = T (either via set_num_workers or a temporary env/compile flag)
     rm -f big-table.csv
     examples/cpathgen/big-table 1000000 benchmarks/leon2.edges golden/leon2.golden
     echo "T=$T:"; cat big-table.csv
   done
   ```
   Compare against the iter 3 thread-scaling table. If contention was the bottleneck, higher thread counts should now yield better speedups than before (the curve should be flatter / scale further).

5. Confirm `drain_count = 0` unchanged on all runs.

**Success criterion:** ≥5% improvement on at least 2 of 3 large benchmarks at 4T, zero error, drain=0. Scalability curve should show meaningful improvement at ≥6T vs iter 3 baseline.

---

## Risk

**Low.** The combinable pattern is proven (used for `local_paths` since iter 1). The type passed to `_spur_tbb_task_vecs` is unchanged. The only new failure mode is a bug in the `combine_each` flush (e.g., off-by-one in `grow_by`) — straightforward to test and debug.

**Revert path:** `git stash` or `git checkout ink/ink.cpp` back to iter 3 checkpoint if correctness fails after 3 fix attempts (per CLAUDE.md protocol).
