# Flush Scan Reduction + Queue Count Tuning (iter6) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Eliminate the T=6 regression from iter5 and improve 4T performance by (A) starting the `combine_each` flush scan at `id` instead of 0, and (B) reducing `num_task_qs` from 100 to the smallest correct value.

**Architecture:** Approach A exploits cost monotonicity (child cost ≥ parent cost → bands 0..id-1 are provably empty during band-id processing) for a one-line change. Approach B reduces worst-case scan width at early bands, compounding the savings. Both are independent and can be reverted independently.

**Tech Stack:** C++20, TBB, CMake, `examples/cpathgen/big-table` for verification.

---

## File Map

| File | Change |
|------|--------|
| `ink/ink.cpp:1947-1955` | Update stale comment + change `qi = 0` → `qi = id` in `combine_each` flush |
| `examples/cpathgen/big-table.cpp:86` | Sweep `num_queues` argument (100 → candidate Q) |

No other files are modified. `_spur_tbb_task_vecs` and all other functions are untouched.

---

## Task 1: Confirm iter5 baseline numbers are recorded

**Files:** none (data check only)

- [ ] **Step 1: Verify iter5-tl-task-vecs tag exists**

```bash
git tag | grep iter5
```

Expected: `iter5-tl-task-vecs` in output. If missing, tag current HEAD:
```bash
git tag iter5-tl-task-vecs
```

- [ ] **Step 2: Confirm iter5 numbers are in the experiment log**

```bash
grep -A 30 "Iteration 5" debug_session/experiment_log.md
```

Expected: T=4 and scalability rows filled in. The iter5 reference numbers are:
- leon2 4T: 252.7ms, leon3mp 4T: 207.8ms, netcard 4T: 226.1ms
- Scalability (leon2): T=2: 324.9ms, T=4: 242.3ms, T=6: 367.0ms, T=8: 268.5ms, T=10: 274.6ms

If the log is missing these rows, run the baseline first (checkout iter5-tl-task-vecs, build, measure, then return to HEAD).

---

## Task 2: Apply Approach A — start flush scan from `id`

**Files:**
- Modify: `ink/ink.cpp:1947-1955`

- [ ] **Step 1: Read the current flush block**

Open `ink/ink.cpp` lines 1947–1962. Confirm line 1947 reads:
```
// Flush per-thread child buffers into global task_vecs.
```
Confirm the stale comment at lines 1948–1949 reads:
```
// Children may land in any band (0..num_task_qs-1), including already-processed
// or not-yet-reached bands — the flush correctly covers all qi.
```
And confirm line 1955 reads:
```cpp
for (size_t qi = 0; qi < num_task_qs; qi++) {
```

- [ ] **Step 2: Apply the change**

Replace lines 1948–1955 only (line 1947 is a header line kept unchanged). Change from:
```cpp
          // Children may land in any band (0..num_task_qs-1), including already-processed
          // or not-yet-reached bands — the flush correctly covers all qi.
          // clear() is called AFTER std::move, so all unique_ptrs are already null/moved-from
          // before clear() runs — safe. tbb::concurrent_vector::clear() does not release
          // segment memory, but per-thread footprint is bounded (num_task_qs bands) and
          // stable after the first few windows — the retained segments are reused next window.
          tl_task_vecs.combine_each([&](auto& local_tv) {
            for (size_t qi = 0; qi < num_task_qs; qi++) {
```
to:
```cpp
          // Children may land in bands >= id (cost monotonicity: child cost >= parent cost).
          // Bands 0..id-1 are provably empty — scan starts at id.
          // clear() is called AFTER std::move, so all unique_ptrs are already null/moved-from
          // before clear() runs — safe. tbb::concurrent_vector::clear() does not release
          // segment memory, but per-thread footprint is bounded (num_task_qs bands) and
          // stable after the first few windows — the retained segments are reused next window.
          tl_task_vecs.combine_each([&](auto& local_tv) {
            for (size_t qi = id; qi < num_task_qs; qi++) {
```

- [ ] **Step 3: Build**

```bash
cmake --build build --parallel 2>&1 | tail -10
```

Expected: zero errors.

- [ ] **Step 4: Correctness check (K=1M, all 4 large benchmarks)**

```bash
rm -f big-table.csv
for bm in vga_lcd leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

**Must pass:** `pathgen_avg_err=0.0` and `pathgen_max_err=0.0` on all four rows, `drain_count=0`.

If any error > 0.0: the scan is skipping non-empty bands. Check that `id` is captured by `[&]` in the enclosing lambda and that the `for` loop correctly starts at `id`. Do not proceed to Step 5 until correctness passes. Max 3 fix attempts; if still failing, revert:
```bash
git checkout ink/ink.cpp
cmake --build build --parallel
```

- [ ] **Step 5: Scalability sweep (leon2, K=1M, T=2/4/6/8/10)**

```bash
for T in 2 4 6 8 10; do
  rm -f big-table.csv
  examples/cpathgen/big-table 1000000 benchmarks/leon2.edges golden/leon2.golden $T
  echo "T=$T:" && cat big-table.csv
done
```

Expected: T=6 regression gone or greatly reduced (< 5% vs iter5 T=4). T=4 same or better than iter5 (252.7ms).

- [ ] **Step 6: Commit and tag**

```bash
git add ink/ink.cpp
git commit -m "perf: start combine_each flush scan at id (cost monotonicity, iter6-A)"
git tag iter6-id-start
```

---

## Task 3: Apply Approach B — tune num_task_qs

**Files:**
- Modify: `examples/cpathgen/big-table.cpp:86`

This task sweeps Q values to find the smallest value that preserves correctness on all three large benchmarks. Each candidate requires an edit + rebuild + correctness check.

> **Important:** If any Q value produces non-zero error, **stop immediately** — this is a correctness bug in the algorithm, not a tuning artifact. Do not fall back to a larger Q. Diagnose and fix the bug before continuing.

- [ ] **Step 1: Verify iter6-id-start tag exists before proceeding**

```bash
git tag | grep iter6
```

Expected: `iter6-id-start`. If missing, complete Task 2 first.

- [ ] **Step 2: Q=100 — confirm baseline still passes (no edit needed)**

`big-table.cpp` currently has `100,` at line 86 from before iter6. Verify correctness at this value first before reducing:

```bash
cmake --build build --parallel 2>&1 | tail -5
rm -f big-table.csv
for bm in leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

**Must pass:** `pathgen_avg_err=0.0`, `pathgen_max_err=0.0` on all three rows. This is the known-good baseline — if it fails, stop and investigate before proceeding.

- [ ] **Step 3: Q=50 — edit, build, verify all 3 benchmarks**

Edit `examples/cpathgen/big-table.cpp` line 86: change `100,` to `50,`.

```bash
cmake --build build --parallel 2>&1 | tail -5
rm -f big-table.csv
for bm in leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

**Must pass:** `pathgen_avg_err=0.0`, `pathgen_max_err=0.0` on all three rows.

If error > 0.0 on any benchmark: stop — correctness bug, do not try smaller Q. Revert big-table.cpp and investigate the algorithm.

- [ ] **Step 4: Q=20 — edit, build, verify all 3 benchmarks**

Edit `examples/cpathgen/big-table.cpp` line 86: change to `20,`.

```bash
cmake --build build --parallel 2>&1 | tail -5
rm -f big-table.csv
for bm in leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

Same correctness requirement. If error > 0.0: stop and investigate.

- [ ] **Step 5: Q=10 — edit, build, verify all 3 benchmarks**

Edit `examples/cpathgen/big-table.cpp` line 86: change to `10,`.

```bash
cmake --build build --parallel 2>&1 | tail -5
rm -f big-table.csv
for bm in leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

Same correctness requirement. If error > 0.0: stop and investigate.

- [ ] **Step 6: Choose Q and set final value**

Pick the smallest Q from {50, 20, 10} where all three benchmarks show `pathgen_avg_err=0.0` and `pathgen_max_err=0.0`. Edit `big-table.cpp:86` to that value if not already set.

If all three pass at Q=10, use Q=10.
If Q=10 fails but Q=20 passes, use Q=20.
If Q=20 fails but Q=50 passes, use Q=50.

- [ ] **Step 7: Scalability sweep at chosen Q (leon2, K=1M, T=2/4/6/8/10)**

```bash
for T in 2 4 6 8 10; do
  rm -f big-table.csv
  examples/cpathgen/big-table 1000000 benchmarks/leon2.edges golden/leon2.golden $T
  echo "T=$T:" && cat big-table.csv
done
```

- [ ] **Step 8: Commit and tag**

```bash
git add examples/cpathgen/big-table.cpp
git commit -m "perf: reduce num_task_qs to Q (flush scan reduction, iter6-B)"
git tag iter6-tuned-queues
```

(Replace `Q` in the commit message with the chosen value.)

---

## Task 4: Final measurement and experiment log

**Files:**
- Modify: `debug_session/experiment_log.md`

- [ ] **Step 1: Run final 3-benchmark comparison at T=4 (default)**

```bash
rm -f big-table.csv
for bm in leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

Record `pathgen_avg_pfxt_time` for each. Compare vs iter5 4T baseline: leon2=252.7ms, leon3mp=207.8ms, netcard=226.1ms.

- [ ] **Step 2: Append iteration 6 results to experiment log**

Append to `debug_session/experiment_log.md`:

```markdown
## Iteration 6

**Optimizations:** flush scan start from `id` (cost monotonicity) + num_task_qs reduced to Q

### Outcome (K=1M, 5-run avg, T=4)

| Benchmark | iter5 4T (ms) | iter6 4T (ms) | Δ% |
|-----------|--------------|--------------|-----|
| leon2     | 252.7        | <fill>       | <fill> |
| leon3mp   | 207.8        | <fill>       | <fill> |
| netcard   | 226.1        | <fill>       | <fill> |

### Scalability (leon2, K=1M)

| threads | iter5 (ms) | iter6 (ms) | Δ% |
|---------|-----------|-----------|-----|
| 2       | 324.9     | <fill>    | <fill> |
| 4       | 242.3     | <fill>    | <fill> |
| 6       | 367.0     | <fill>    | <fill> |
| 8       | 268.5     | <fill>    | <fill> |
| 10      | 274.6     | <fill>    | <fill> |

drain_count: <fill> (must be 0)
pathgen_avg_err: <fill> (must be 0.0)
num_task_qs chosen: <fill>
```

Fill in all `<fill>` values from the measurements above.

- [ ] **Step 3: Commit experiment log**

```bash
git add debug_session/experiment_log.md
git commit -m "results: iter6 flush scan reduction scalability measurements"
```

---

## Quick Reference: Revert Paths

```bash
# Revert A only (keep big-table.cpp changes):
git checkout iter5-tl-task-vecs -- ink/ink.cpp
cmake --build build --parallel

# Revert B only (requires iter6-id-start tag — must commit after Task 2 first):
git checkout iter6-id-start -- examples/cpathgen/big-table.cpp
cmake --build build --parallel

# Revert both:
git checkout iter5-tl-task-vecs -- ink/ink.cpp examples/cpathgen/big-table.cpp
cmake --build build --parallel
```
