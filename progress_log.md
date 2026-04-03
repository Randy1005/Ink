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

---
## Round 4 — static delta=1.5

**Phase 1** | Config ID 4

### Parameters

| param | value |
|---|---|
| delta_init | 1.5 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 65.1 | 0.0000 | 0.0000 | 1 | 1468888 | 1.500 | 1.500 |
| leon3mp | 50.1 | 0.0000 | 0.0000 | 1 | 1132948 | 1.500 | 1.500 |
| netcard | 61.9 | 0.0000 | 0.0000 | 1 | 1355772 | 1.500 | 1.500 |

**Avg time across benchmarks:** 59.0 ms

**Verdict:** 🔻 regression   (51.5 → 59.0 ms avg)

**Best so far:** static delta=0.5 @ 51.46666666666667 ms

---
## Round 5 — static delta=2.0

**Phase 1** | Config ID 5

### Parameters

| param | value |
|---|---|
| delta_init | 2.0 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 55.3 | 0.0000 | 0.0000 | 1 | 1299420 | 2.000 | 2.000 |
| leon3mp | 43.1 | 0.0000 | 0.0000 | 1 | 1001296 | 2.000 | 2.000 |
| netcard | 52.2 | 0.0000 | 0.0000 | 1 | 1200332 | 2.000 | 2.000 |

**Avg time across benchmarks:** 50.2 ms

**Verdict:** ➖ plateau      (50.2 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 6 — static delta=2.5

**Phase 1** | Config ID 6

### Parameters

| param | value |
|---|---|
| delta_init | 2.5 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 69.2 | 0.0000 | 0.0000 | 1 | 1635380 | 2.500 | 2.500 |
| leon3mp | 54.0 | 0.0000 | 0.0000 | 1 | 1260400 | 2.500 | 2.500 |
| netcard | 63.6 | 0.0000 | 0.0000 | 1 | 1506344 | 2.500 | 2.500 |

**Avg time across benchmarks:** 62.3 ms

**Verdict:** 🔻 regression   (50.2 → 62.3 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 7 — static delta=3.5

**Phase 1** | Config ID 7

### Parameters

| param | value |
|---|---|
| delta_init | 3.5 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 44.3 | 0.0000 | 0.0000 | 1 | 1124444 | 3.500 | 3.500 |
| leon3mp | 74.4 | 0.0000 | 0.0000 | 1 | 1753152 | 3.500 | 3.500 |
| netcard | 39.9 | 0.0000 | 0.0000 | 1 | 1039476 | 3.500 | 3.500 |

**Avg time across benchmarks:** 52.9 ms

**Verdict:** 🔻 regression   (50.2 → 52.9 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 8 — static delta=5.0

**Phase 1** | Config ID 8

### Parameters

| param | value |
|---|---|
| delta_init | 5.0 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 63.9 | 0.0000 | 0.0000 | 1 | 1635380 | 5.000 | 5.000 |
| leon3mp | 49.3 | 0.0000 | 0.0000 | 1 | 1260400 | 5.000 | 5.000 |
| netcard | 58.3 | 0.0000 | 0.0000 | 1 | 1506344 | 5.000 | 5.000 |

**Avg time across benchmarks:** 57.2 ms

**Verdict:** 🔻 regression   (50.2 → 57.2 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 9 — static delta=10.0

**Phase 1** | Config ID 9

### Parameters

| param | value |
|---|---|
| delta_init | 10.0 |
| target_pps | 0.0 |
| scale_up | 1.0 |
| scale_down | 1.0 |
| delta_min | 0.1 |
| delta_max | 100.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 130.3 | 0.0000 | 0.0000 | 1 | 3142768 | 10.000 | 10.000 |
| leon3mp | 96.9 | 0.0000 | 0.0000 | 1 | 2428584 | 10.000 | 10.000 |
| netcard | 113.9 | 0.0000 | 0.0000 | 1 | 2855388 | 10.000 | 10.000 |

**Avg time across benchmarks:** 113.7 ms

**Verdict:** 🔻 regression   (50.2 → 113.7 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 10 — adaptive d0=0.5 tpps=50000 up=1.5 dn=0.7

**Phase 2** | Config ID 10

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 50000.0 |
| scale_up | 1.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 54.5 | 0.0000 | 0.0000 | 1 | 1124444 | 0.300 | 0.300 |
| leon3mp | 48.3 | 0.0000 | 0.0000 | 1 | 1001296 | 0.300 | 0.300 |
| netcard | 51.0 | 0.0000 | 0.0000 | 1 | 1039476 | 0.300 | 0.300 |

**Avg time across benchmarks:** 51.3 ms

**Verdict:** 🔻 regression   (50.2 → 51.3 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 11 — adaptive d0=0.5 tpps=50000 up=1.5 dn=0.9

**Phase 2** | Config ID 11

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 50000.0 |
| scale_up | 1.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 55.6 | 0.0000 | 0.0000 | 1 | 1124444 | 0.400 | 0.400 |
| leon3mp | 49.4 | 0.0000 | 0.0000 | 1 | 1001296 | 0.400 | 0.400 |
| netcard | 50.9 | 0.0000 | 0.0000 | 1 | 1039476 | 0.400 | 0.400 |

**Avg time across benchmarks:** 52.0 ms

**Verdict:** 🔻 regression   (50.2 → 52.0 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 12 — adaptive d0=0.5 tpps=50000 up=2.5 dn=0.7

**Phase 2** | Config ID 12

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 50000.0 |
| scale_up | 2.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 56.1 | 0.0000 | 0.0000 | 1 | 1124444 | 0.300 | 0.300 |
| leon3mp | 49.8 | 0.0000 | 0.0000 | 1 | 1001296 | 0.300 | 0.300 |
| netcard | 50.7 | 0.0000 | 0.0000 | 1 | 1039476 | 0.300 | 0.300 |

**Avg time across benchmarks:** 52.2 ms

**Verdict:** 🔻 regression   (50.2 → 52.2 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 13 — adaptive d0=0.5 tpps=50000 up=2.5 dn=0.9

**Phase 2** | Config ID 13

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 50000.0 |
| scale_up | 2.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 54.4 | 0.0000 | 0.0000 | 1 | 1124444 | 0.400 | 0.400 |
| leon3mp | 50.5 | 0.0000 | 0.0000 | 1 | 1001296 | 0.400 | 0.400 |
| netcard | 50.5 | 0.0000 | 0.0000 | 1 | 1039476 | 0.400 | 0.400 |

**Avg time across benchmarks:** 51.8 ms

**Verdict:** 🔻 regression   (50.2 → 51.8 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 14 — adaptive d0=0.5 tpps=200000 up=1.5 dn=0.7

**Phase 2** | Config ID 14

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 200000.0 |
| scale_up | 1.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 55.8 | 0.0000 | 0.0000 | 1 | 1124444 | 0.300 | 0.300 |
| leon3mp | 48.1 | 0.0000 | 0.0000 | 1 | 1001296 | 0.300 | 0.300 |
| netcard | 55.6 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 53.2 ms

**Verdict:** 🔻 regression   (50.2 → 53.2 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 15 — adaptive d0=0.5 tpps=200000 up=1.5 dn=0.9

**Phase 2** | Config ID 15

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 200000.0 |
| scale_up | 1.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 56.7 | 0.0000 | 0.0000 | 1 | 1124444 | 0.500 | 0.500 |
| leon3mp | 49.5 | 0.0000 | 0.0000 | 1 | 1001296 | 0.500 | 0.500 |
| netcard | 52.1 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 52.8 ms

**Verdict:** 🔻 regression   (50.2 → 52.8 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 16 — adaptive d0=0.5 tpps=200000 up=2.5 dn=0.7

**Phase 2** | Config ID 16

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 200000.0 |
| scale_up | 2.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 53.8 | 0.0000 | 0.0000 | 1 | 1124444 | 0.500 | 0.500 |
| leon3mp | 48.7 | 0.0000 | 0.0000 | 1 | 1001296 | 0.500 | 0.500 |
| netcard | 50.1 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 50.9 ms

**Verdict:** 🔻 regression   (50.2 → 50.9 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 17 — adaptive d0=0.5 tpps=200000 up=2.5 dn=0.9

**Phase 2** | Config ID 17

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 200000.0 |
| scale_up | 2.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 55.6 | 0.0000 | 0.0000 | 1 | 1124444 | 0.500 | 0.500 |
| leon3mp | 49.6 | 0.0000 | 0.0000 | 1 | 1001296 | 0.500 | 0.500 |
| netcard | 51.6 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 52.3 ms

**Verdict:** 🔻 regression   (50.2 → 52.3 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 18 — adaptive d0=0.5 tpps=500000 up=1.5 dn=0.7

**Phase 2** | Config ID 18

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 500000.0 |
| scale_up | 1.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 54.9 | 0.0000 | 0.0000 | 1 | 1124444 | 0.500 | 0.500 |
| leon3mp | 49.6 | 0.0000 | 0.0000 | 1 | 1001296 | 0.500 | 0.500 |
| netcard | 49.3 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 51.3 ms

**Verdict:** 🔻 regression   (50.2 → 51.3 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 19 — adaptive d0=0.5 tpps=500000 up=1.5 dn=0.9

**Phase 2** | Config ID 19

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 500000.0 |
| scale_up | 1.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 55.2 | 0.0000 | 0.0000 | 1 | 1124444 | 0.500 | 0.500 |
| leon3mp | 49.8 | 0.0000 | 0.0000 | 1 | 1001296 | 0.500 | 0.500 |
| netcard | 50.3 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 51.8 ms

**Verdict:** 🔻 regression   (50.2 → 51.8 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 20 — adaptive d0=0.5 tpps=500000 up=2.5 dn=0.7

**Phase 2** | Config ID 20

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 500000.0 |
| scale_up | 2.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 54.9 | 0.0000 | 0.0000 | 1 | 1124444 | 0.500 | 0.500 |
| leon3mp | 48.1 | 0.0000 | 0.0000 | 1 | 1001296 | 0.500 | 0.500 |
| netcard | 51.5 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 51.5 ms

**Verdict:** 🔻 regression   (50.2 → 51.5 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 21 — adaptive d0=0.5 tpps=500000 up=2.5 dn=0.9

**Phase 2** | Config ID 21

### Parameters

| param | value |
|---|---|
| delta_init | 0.5 |
| target_pps | 500000.0 |
| scale_up | 2.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 55.4 | 0.0000 | 0.0000 | 1 | 1124444 | 0.500 | 0.500 |
| leon3mp | 48.5 | 0.0000 | 0.0000 | 1 | 1001296 | 0.500 | 0.500 |
| netcard | 50.4 | 0.0000 | 0.0000 | 1 | 1039476 | 0.500 | 0.500 |

**Avg time across benchmarks:** 51.4 ms

**Verdict:** 🔻 regression   (50.2 → 51.4 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 22 — adaptive d0=1.0 tpps=50000 up=1.5 dn=0.7

**Phase 2** | Config ID 22

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 50000.0 |
| scale_up | 1.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 59.9 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 45.2 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 56.3 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 53.8 ms

**Verdict:** 🔻 regression   (50.2 → 53.8 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 23 — adaptive d0=1.0 tpps=50000 up=1.5 dn=0.9

**Phase 2** | Config ID 23

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 50000.0 |
| scale_up | 1.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 60.1 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 46.5 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 56.2 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 54.3 ms

**Verdict:** 🔻 regression   (50.2 → 54.3 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 24 — adaptive d0=1.0 tpps=50000 up=2.5 dn=0.7

**Phase 2** | Config ID 24

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 50000.0 |
| scale_up | 2.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 59.5 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 46.1 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 54.6 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 53.4 ms

**Verdict:** 🔻 regression   (50.2 → 53.4 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 25 — adaptive d0=1.0 tpps=50000 up=2.5 dn=0.9

**Phase 2** | Config ID 25

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 50000.0 |
| scale_up | 2.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 60.8 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 46.4 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 56.1 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 54.4 ms

**Verdict:** 🔻 regression   (50.2 → 54.4 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 26 — adaptive d0=1.0 tpps=200000 up=1.5 dn=0.7

**Phase 2** | Config ID 26

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 200000.0 |
| scale_up | 1.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 59.5 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 45.9 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 54.7 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 53.4 ms

**Verdict:** 🔻 regression   (50.2 → 53.4 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 27 — adaptive d0=1.0 tpps=200000 up=1.5 dn=0.9

**Phase 2** | Config ID 27

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 200000.0 |
| scale_up | 1.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 59.6 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 46.2 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 56.0 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 53.9 ms

**Verdict:** 🔻 regression   (50.2 → 53.9 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 28 — adaptive d0=1.0 tpps=200000 up=2.5 dn=0.7

**Phase 2** | Config ID 28

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 200000.0 |
| scale_up | 2.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 60.5 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 46.2 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 56.4 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 54.4 ms

**Verdict:** 🔻 regression   (50.2 → 54.4 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 29 — adaptive d0=1.0 tpps=200000 up=2.5 dn=0.9

**Phase 2** | Config ID 29

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 200000.0 |
| scale_up | 2.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 60.8 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 45.5 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 55.9 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 54.1 ms

**Verdict:** 🔻 regression   (50.2 → 54.1 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 30 — adaptive d0=1.0 tpps=500000 up=1.5 dn=0.7

**Phase 2** | Config ID 30

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 500000.0 |
| scale_up | 1.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 61.1 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 45.3 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 55.9 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 54.1 ms

**Verdict:** 🔻 regression   (50.2 → 54.1 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 31 — adaptive d0=1.0 tpps=500000 up=1.5 dn=0.9

**Phase 2** | Config ID 31

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 500000.0 |
| scale_up | 1.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 59.8 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 46.1 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 55.8 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 53.9 ms

**Verdict:** 🔻 regression   (50.2 → 53.9 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 32 — adaptive d0=1.0 tpps=500000 up=2.5 dn=0.7

**Phase 2** | Config ID 32

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 500000.0 |
| scale_up | 2.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 59.5 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 46.4 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 55.0 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 53.6 ms

**Verdict:** 🔻 regression   (50.2 → 53.6 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 33 — adaptive d0=1.0 tpps=500000 up=2.5 dn=0.9

**Phase 2** | Config ID 33

### Parameters

| param | value |
|---|---|
| delta_init | 1.0 |
| target_pps | 500000.0 |
| scale_up | 2.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 61.0 | 0.0000 | 0.0000 | 1 | 1299420 | 1.000 | 1.000 |
| leon3mp | 45.3 | 0.0000 | 0.0000 | 1 | 1001296 | 1.000 | 1.000 |
| netcard | 56.0 | 0.0000 | 0.0000 | 1 | 1200332 | 1.000 | 1.000 |

**Avg time across benchmarks:** 54.1 ms

**Verdict:** 🔻 regression   (50.2 → 54.1 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 34 — adaptive d0=2.0 tpps=50000 up=1.5 dn=0.7

**Phase 2** | Config ID 34

### Parameters

| param | value |
|---|---|
| delta_init | 2.0 |
| target_pps | 50000.0 |
| scale_up | 1.5 |
| scale_down | 0.7 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 57.3 | 0.0000 | 0.0000 | 1 | 1299420 | 2.000 | 2.000 |
| leon3mp | 43.4 | 0.0000 | 0.0000 | 1 | 1001296 | 2.000 | 2.000 |
| netcard | 53.5 | 0.0000 | 0.0000 | 1 | 1200332 | 2.000 | 2.000 |

**Avg time across benchmarks:** 51.4 ms

**Verdict:** 🔻 regression   (50.2 → 51.4 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms

---
## Round 35 — adaptive d0=2.0 tpps=50000 up=1.5 dn=0.9

**Phase 2** | Config ID 35

### Parameters

| param | value |
|---|---|
| delta_init | 2.0 |
| target_pps | 50000.0 |
| scale_up | 1.5 |
| scale_down | 0.9 |
| delta_min | 0.1 |
| delta_max | 50.0 |

### Results (K=1M, 16 threads)

| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |
|---|---|---|---|---|---|---|---|
| leon2 | 56.7 | 0.0000 | 0.0000 | 1 | 1299420 | 2.000 | 2.000 |
| leon3mp | 44.3 | 0.0000 | 0.0000 | 1 | 1001296 | 2.000 | 2.000 |
| netcard | 53.6 | 0.0000 | 0.0000 | 1 | 1200332 | 2.000 | 2.000 |

**Avg time across benchmarks:** 51.5 ms

**Verdict:** 🔻 regression   (50.2 → 51.5 ms avg)

**Best so far:** static delta=2.0 @ 50.20000000000001 ms
