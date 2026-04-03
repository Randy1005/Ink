# cpathgen Delta Optimization — Progress Log

Autonomous search for the optimal adaptive delta strategy in `_spur_mlq`.

**Lever:** `delta` (step size / threshold increment) — static or dynamically adjusted
per-step based on paths generated.

**Adaptation rule:** `f(paths_this_step, target_pps) → new_delta`
- `paths_this_step < target_pps` → `delta *= scale_up`  (sparse step, grow window)
- `paths_this_step > target_pps` → `delta *= scale_down` (dense step, shrink window)
- `target_pps == 0` → static delta (no adaptation)

**Metric:** cpathgen `pfxt_time` (ms), averaged over leon2 / leon3mp / netcard at K=1M.
**Correctness gate:** `avg_err == 0.0` and `max_err == 0.0` — required before any
timing result counts.

**Thread count:** 16 (optimal for this 20-thread machine; see thread sweep in session log)

---

## Baseline — static delta=2.5 (pre-autoloop)

Recorded before search began. Hardware: Linux x86-64, 20-thread machine.

| benchmark | time_ms | avg_err | max_err | steps | avg_pps  |
|-----------|---------|---------|---------|-------|----------|
| leon2     | 71.2    | 0.0     | 0.0     | 1     | 1,635,380 |
| leon3mp   | —       | —       | —       | —     | —        |
| netcard   | —       | —       | —       | —     | —        |

> Note: leon3mp and netcard baseline times not recorded individually before autoloop;
> will be established in Round 1 (static delta=2.5 is config ID 6 in Phase 1).

**Observation:** With delta=2.5 and 10 queues, leon2 K=1M finishes in a single step
(all 1M paths fit in the initial active cost bands — no bounds bump needed).
This is the ideal case. Dynamic delta is most impactful when the distribution forces
multiple steps; Phase 1 static sweep will map where the 1-step boundary lies.

---

*Rounds below are appended automatically by `scripts/autoloop.py`.*

---
## Round 1 — static delta=0.1

**Phase 1** | Config ID 1

### Parameters

| param | value |
|---|---|
| delta_init | 0.1 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 126.7 | 0.0000 | 0.0000 | 4 | 254294 | 0.100 | 0.100 |
| leon3mp | 122.7 | 0.0000 | 0.0000 | 5 | 200259 | 0.100 | 0.100 |
| netcard | 120.9 | 0.0000 | 0.0000 | 4 | 251647 | 0.100 | 0.100 |

**Avg time across benchmarks:** 123.4 ms

**Verdict:** ➖ plateau      (123.4 ms avg)

**Best so far:** static delta=0.1 @ 123.43333333333334 ms

---
## Round 2 — static delta=0.5

**Phase 1** | Config ID 2

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 54.6 | 0.0000 | 0.0000 | 1 | 1124444 | 0.500 | 0.500 |
| leon3mp | 49.7 | 0.0000 | 0.0000 | 1 | 1001296 | 0.500 | 0.500 |
| netcard | 50.1 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 51.5 ms

**Verdict:** ➖ plateau      (51.5 ms avg)

**Best so far:** static delta=0.5 @ 51.46666666666667 ms

---
## Round 3 — static delta=1.0

**Phase 1** | Config ID 3

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 60.1 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 45.9 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 55.6 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 53.9 ms

**Verdict:** 🔻 regression   (51.5 → 53.9 ms avg)

**Best so far:** static delta=0.5 @ 51.46666666666667 ms
