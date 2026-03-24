# Design: Flush Scan Reduction + Queue Count Tuning for `_spur_multiq`

**Date:** 2026-03-24
**Status:** Approved
**Target function:** `Ink::_spur_multiq` (`ink/ink.cpp:1865`)
**Builds on:** iter5 (per-thread task_vecs, `docs/superpowers/specs/2026-03-24-spur-multiq-per-thread-task-vecs.md`)

---

## Problem

iter5 introduced per-thread `tl_task_vecs` buffering that eliminates concurrent push_back contention. At T=4 it gives +6–34% improvement. However at T=6 it produces a **-42% regression** because the `combine_each` flush scans all `num_task_qs` (100) bands per thread per window regardless of which are empty. At T=6 this is 600 band checks per window. The overhead dominates the contention savings.

Two independent fixes address this:

- **A:** Start the flush scan at `id` (the current band index) instead of 0 — provably correct shortcut eliminating half the scan on average.
- **B:** Reduce `num_task_qs` from 100 to ~20 — makes the worst-case scan 5× cheaper.

---

## Approach A: Start combine_each Scan from `id`

### Change

In `_spur_multiq`, inside the `tl_task_vecs.combine_each` flush, change the loop start:

```cpp
// Before (iter5):
for (size_t qi = 0; qi < num_task_qs; qi++) {

// After:
for (size_t qi = id; qi < num_task_qs; qi++) {
```

`id` is the current outer-loop band index, already in scope via `[&]` capture.

### Correctness

**Monotonicity derivation:** For any spur edge (u→v, weight w), the detour cost increment is `dists[v] + w - dists[u]`. By the SSSP triangle inequality, `dists[v] + w ≥ dists[u]` — if it were less, the SSSP would have routed through (u,v), contradicting optimality of `dists[u]`. Therefore the detour increment is always ≥ 0, and every child's cost ≥ its parent's cost. Note: overall path costs can be negative (e.g., `vga_lcd` has negative-cost golden paths) but the *incremental detour cost* is always non-negative, which is what matters here.

**id=0 edge case:** At `id=0`, the scan starts at `qi=0` — identical to the existing behaviour. No bands are skipped. This is correct and safe.

**Overflow band:** The outer loop (`for id = 0; id < num_task_qs-1`) never sets `id = num_task_qs-1`. The overflow band is handled separately by the overflow promotion block below the loop. The `combine_each` flush inside the active-band loop never runs for the overflow band, so there is no interaction.

**Main argument:** A parent in band `id` has cost in `(bounds[id-1], bounds[id]]` (or `≤ bounds[0]` at id=0). All children have cost ≥ parent cost, so they land in bands `>= id`. Bands `0..id-1` receive zero children during band `id`'s processing — provably empty in every thread's local copy. Skipping them is always safe.

### Expected Impact

| Band | Bands scanned before | Bands scanned after |
|------|---------------------|---------------------|
| id=0  | 100 | 100 (no change) |
| id=25 | 100 | 75  (-25%) |
| id=50 | 100 | 50  (-50%) |
| id=75 | 100 | 25  (-75%) |
| id=90 | 100 | 10  (-90%) |

Average reduction: ~50%. At T=6 the per-window overhead drops from 600 to ~300 band checks on average. The T=6 regression should largely disappear.

### Scope

One line change in `ink/ink.cpp` (loop variable `0` → `id`) plus updating the stale comment at line ~1948 that currently reads:

> "Children may land in any band (0..num_task_qs-1), including already-processed or not-yet-reached bands — the flush correctly covers all qi."

This comment was written conservatively before the monotonicity argument was formally established. It must be corrected to:

> "Children may land in bands >= id (cost monotonicity: child cost >= parent cost). Bands 0..id-1 are provably empty — scan starts at id."

No other files are touched by step A.

---

## Approach B: Reduce `num_task_qs` (Tuning Pass)

### Motivation

After A, the worst-case scan at `id=0` is still `num_task_qs` bands. With 100 bands, band 0 still scans 100 slots per thread. Reducing to 20 bands cuts this to 20 — 5× cheaper across all early bands.

### Change

In `examples/cpathgen/big-table.cpp`, change the `num_queues` argument passed to `report_multiq`:

```cpp
// Before:
pathgen.report_multiq(max_cost, min_cost, k, 100, false, true, false);

// After (candidate):
pathgen.report_multiq(max_cost, min_cost, k, 20, false, true, false);
```

### Tuning Protocol

For each candidate Q, manually edit `big-table.cpp` to replace the `100` argument, rebuild, then verify all three large benchmarks (leon2, leon3mp, netcard) — not just leon2, since different graphs have different cost distributions and a Q that's correct on leon2 may fail on netcard:

```bash
# Pseudocode — each iteration requires: edit big-table.cpp, cmake --build build --parallel, then run:
for Q in 100 50 20 10; do
  # 1. Edit big-table.cpp: change 100 → Q in the report_multiq call
  # 2. cmake --build build --parallel
  rm -f big-table.csv
  for bm in leon2 leon3mp netcard; do
    examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden 4
  done
  echo "=== Q=$Q ===" && cat big-table.csv
done
```

Choose the smallest Q where `pathgen_avg_err=0.0` and `pathgen_max_err=0.0` on **all three** benchmarks. If error appears at any Q, **do not fall back to a larger value** — non-zero error indicates a correctness bug in the algorithm that must be diagnosed and fixed before proceeding. Error is never a tuning artefact.

### Risk

Too few bands → wider cost buckets → more overflow promotions. The EQUAL policy's analytical bump makes overflow O(1) per cycle, so moderate reduction is safe. If any Q causes non-zero error, stop immediately — this is a correctness bug, not a tuning issue, and must be investigated before continuing.

### Scope

`examples/cpathgen/big-table.cpp` only. `_spur_multiq` is unchanged by this step.

---

## Implementation Order

1. **Apply A** (one-line change + comment update in `ink/ink.cpp`) → build → correctness check → scalability sweep → **commit** → tag `iter6-id-start`
2. **Apply B** (tune `big-table.cpp`, starting from the iter6-id-start commit) → sweep Q values → pick best → verify correctness → scalability sweep → **commit** → tag `iter6-tuned-queues`
3. **Final measurement** — full 3-benchmark run (leon2/leon3mp/netcard) at T=4 for experiment log

> **Important:** Commit and tag after step A before beginning step B. The step B revert (`git checkout iter6-id-start -- examples/cpathgen/big-table.cpp`) requires this tag to exist.

---

## Verification Plan

### After Step A

1. Build: `cmake --build build --parallel`

2. Correctness (K=1M, 4 large benchmarks):
   ```bash
   rm -f big-table.csv
   for bm in vga_lcd leon2 leon3mp netcard; do
     examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
   done
   cat big-table.csv
   ```
   `pathgen_avg_err=0.0`, `pathgen_max_err=0.0` on all four. `drain_count=0`.

3. Scalability sweep (leon2, K=1M, T=2/4/6/8/10):
   ```bash
   for T in 2 4 6 8 10; do
     rm -f big-table.csv
     examples/cpathgen/big-table 1000000 benchmarks/leon2.edges golden/leon2.golden $T
     echo "T=$T:" && cat big-table.csv
   done
   ```
   Expected: T=6 regression gone or greatly reduced vs iter5. T=4 same or better than iter5.

4. Tag: `git tag iter6-id-start`

### After Step B

5. Queue count sweep (leon2/leon3mp/netcard, K=1M, T=4) — see Tuning Protocol above.
   Pick smallest Q with error=0.0 on all three benchmarks.

6. Scalability sweep at chosen Q (same as step 3 above).

7. Final 3-benchmark run at T=4:
   ```bash
   rm -f big-table.csv
   for bm in leon2 leon3mp netcard; do
     examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
   done
   cat big-table.csv
   ```
   Compare vs iter5 4T baseline: leon2=252.7ms, leon3mp=207.8ms, netcard=226.1ms.

8. Tag: `git tag iter6-tuned-queues`

---

## Success Criteria

- T=6 regression eliminated or reduced to ≤5% vs iter5 (after A)
- T=4 performance same or better than iter5 (after A+B)
- All benchmarks: error=0.0, drain_count=0
- Bonus: scaling curve flatter than iter5 at T≥6

---

## Checkpoints

```bash
git tag iter6-id-start       # after step A
git tag iter6-tuned-queues   # after step B

# Revert A only:
git checkout iter5-tl-task-vecs -- ink/ink.cpp

# Revert B only (requires iter6-id-start tag to exist — commit after step A first):
git checkout iter6-id-start -- examples/cpathgen/big-table.cpp
```
