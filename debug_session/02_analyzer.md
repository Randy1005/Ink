# Concurrency Bug Analysis: `_spur_multiq`

## 1. Primary Root Cause — Early Termination

### Exact Chain

```
while (_atom_path_cnt < K_overgrow)          // line 1903
  └─ submits silent_async task T_i
       └─ T_i calls _spur_multiq(pfxt, node, executor)  // line 1998/2009
            └─ pushes child nodes into pfxt.task_qs[]   // line 2742
                 via pfxt.push_task(...)
```

**Race between writer and condition check:**

1. `_atom_path_cnt` reaches `K_overgrow` (or slightly below it on the last iteration).
2. The `while` condition fails → **no more `silent_async` submissions**.
3. `executor.wait_for_all()` (line 2014) drains only already-submitted tasks.
4. Those running tasks call `_spur_multiq(pfxt, node, executor)` which enqueues fresh children into `pfxt.task_qs[]`.
5. The outer loop has already exited → **those children are never dequeued, never spurred, never enqueued into `paths_concurr`**.
6. If any orphaned child belongs to the true top-K set, it is missing from `_spurred_nodes` → **path cost errors**.

The severity scales with:
- `overgrow_scalar` (how early the loop terminates relative to K)
- Fan-out of the last batch of processed nodes
- How many of those orphans rank inside the true top-K

---

## 2. Secondary Issues Inside `_spur_multiq` Only

### 2a. `_atom_path_cnt` increment is non-atomic relative to the loop guard (line 1903 / 2002 / 2010)

```cpp
// line 2002
_atom_path_cnt += sz;          // bulk path: sz is computed before increment

// line 2010  
_atom_path_cnt++;              // single path
```

`_atom_path_cnt` is `std::atomic` so individual operations are sequentially consistent, but **the guard read and the increment are not a single atomic transaction**. Multiple tasks can all read `_atom_cnt < K_overgrow`, all pass, all increment, causing `_atom_path_cnt` to overshoot by up to `_bulk_size × _num_workers` paths. This makes early termination *worse* (loop exits even earlier than `K_overgrow` demands) and increases the orphan population.

### 2b. `pop_task` (bulk overload) transfers ownership into the task vector but NOT into `paths_concurr` (line 2296–2305 vs. single-node overload line 2281–2293)

Looking at `pop_task` (single node, line 3281): it immediately enqueues the node into `paths_concurr` before returning the raw pointer. The **bulk** `pop_task` (line 3296) only returns the `unique_ptr` vector — ownership transfer to `paths_concurr` is done manually in the lambda (line 1999):

```cpp
pfxt.paths_concurr.enqueue(std::move(n));   // line 1999
```

However, in the `spur_ahead` branch (line 1928), the same pattern is used. This is correct *if* the node is not null. But notice:

```cpp
std::for_each(nodes.begin(), nodes.end(), [&pfxt, &sz, &executor, this](auto& n){
    if (!n) {
        return;          // null node → skipped, NOT enqueued
    }
    sz++;
    _spur_multiq(pfxt, *(n.get()), executor);
    pfxt.paths_concurr.enqueue(std::move(n));
});
_atom_path_cnt += sz;    // line 2002
```

`try_dequeue_bulk` may partially fill the vector (returns fewer than `bulk_size` items), leaving trailing `nullptr`s. The null-guard is correct. **But `sz` counts only non-null nodes, while the vector was pre-allocated to `bulk_size`** (line 3298: `std::vector<std::unique_ptr<PfxtNode>> nodes(bulk_size)`). Trailing default-constructed `unique_ptr`s (null) are silently skipped — this is handled correctly. No secondary bug here.

### 2c. `_tmp_q` redistribution uses a shared member variable without adequate protection (line 1968)

```cpp
_tmp_q = std::move(pfxt.task_qs[q_id]);    // line 1968
```

`_tmp_q` appears to be an `Ink` member (not local). The `redistributing` CAS (line 1963) prevents two threads from entering simultaneously, but between the CAS and the actual move, another thread reading `_tmp_q` for a different purpose (if any) would see torn state. Within `_spur_multiq` this is only written here, so the risk is confined — but it is still a design smell.

### 2d. Condition `!_tmp_q.size_approx() == 0` is logically inverted (line 1971)

```cpp
while (!_tmp_q.size_approx() == 0) {    // line 1971
```

Due to C++ operator precedence, this parses as:

```cpp
while ((!_tmp_q.size_approx()) == 0)
// i.e., while ((_tmp_q.size_approx() == 0) == false ... wait:
// !x == 0  →  (x != 0) ... actually:
// !size_approx() gives 1 when size==0, 0 otherwise
// (1 == 0) → false  when queue IS empty
// (0 == 0) → true   when queue is NOT empty
```

Let's be precise:

| `size_approx()` | `!size_approx()` | `!size_approx() == 0` | Loop continues? |
|---|---|---|---|
| 0 (empty) | 1 | `1 == 0` → **false** | **No → exits** ✓ |
| N>0 (not empty) | 0 | `0 == 0` → **true** | **Yes → continues** ✓ |

This accidentally produces the **correct** behavior, but is highly misleading and fragile. The intended expression is `while (_tmp_q.size_approx() != 0)` or `while (!(_tmp_q.size_approx() == 0))`.

### 2e. `pop_task` single-node path (line 2009) transfers node to `paths_concurr` inside `pop_task` itself, then the node pointer is used after the move

```cpp
// pop_task single (line 3281-3293):
auto obs = node.get();
paths_concurr.enqueue(std::move(node));   // node moved → obs is now a dangling-ish pointer
return obs;                                // raw pointer into moved unique_ptr

// back in _spur_multiq (line 2009):
_spur_multiq(pfxt, *node, executor);      // node == raw ptr returned by pop_task
```

`obs` is still valid as a raw pointer because `paths_concurr.enqueue` stores the `unique_ptr` (the pointed-to object lives), but this design creates an implicit contract: the object must not be destroyed between `pop_task` returning `obs` and `_spur_multiq` finishing with it. Since `paths_concurr` owns the node for the rest of the algorithm, and `_spur_multiq` only reads `pfx.to`, `pfx.cost`, etc., this is *technically* safe — but extremely fragile. Any concurrent dequeue from `paths_concurr` during the spur would be a use-after-free.

---

## 3. Evidence Map

| Line(s) | Issue | Error Mechanism |
|---|---|---|
| **1903** | `while (_atom_path_cnt < K_overgrow)` exits prematurely | Loop stops submitting; in-flight tasks push children that are never consumed |
| **2014** | `executor.wait_for_all()` after early exit | Drains submitted tasks but their newly-pushed children remain in `task_qs` forever |
| **2742** | `pfxt.push_task(...)` inside `_spur_multiq` per-node | Children enqueued here during the "drain" phase are orphaned |
| **1998/2009** | Call to `_spur_multiq(pfxt, node, executor)` inside the async lambda | This is where children are generated *after* the outer loop exits |
| **2002/2010** | `_atom_path_cnt += sz` / `_atom_path_cnt++` | Non-transactional read-check-increment causes overshoot → loop exits even earlier than intended |
| **1971** | `while (!_tmp_q.size_approx() == 0)` | Operator-precedence bug; coincidentally correct but masks intent; could break with type changes |
| **1963–1979** | `_tmp_q` as shared member in redistribution | Shared mutable state; safe only because of `redistributing` CAS, but not obviously so |

---

## 4. Benchmark Correlation

### `aes_core.edges` — highest average error (3.9) and max error (1052.3)

- **66K vertices, 43K edges**: moderate graph, meaning each node has moderate fan-out
- The pfxt tree grows quickly; K paths are found fast → `_atom_path_cnt` hits `K_overgrow` early in wall-clock time, while many in-flight tasks are still generating children
- High max error (1052.3) indicates that some **very-low-cost paths** (which should be in top-K) are orphaned — these are nodes generated late by spur operations that completed just as the loop exited
- The `_atom_path_cnt` overshoot (bug 2a) is amplified with bulk processing: at `_bulk_size > 1`, each task increments by up to 8 at once, causing the loop to stop far sooner than needed

### `des_perf.edges` — second highest max error (343.5), low average (0.7)

- **295K vertices, 189K edges**: large graph with many parallel paths
- Low average error suggests most runs find correct paths, but occasional runs (high max) hit the race condition badly — consistent with a timing-dependent bug
- The large graph means more tasks are in-flight simultaneously when the loop exits → more potential orphans per run

### `vga_lcd.edges` — moderate errors (avg 1.0, max 38.2)

- **397K vertices, 473K edges**: largest graph, but `|E|/|V|` ratio is highest (nearly 1.2)
- Dense fan-out means each spur generates many children → more orphans when early termination hits
- Lower max error than `des_perf` suggests `overgrow_scalar` provides enough buffer for most runs

### `tv80.edges` — near-zero average error (0.0), small max (1.7)

- **16K vertices, 11K edges**: smallest graph
- Paths are exhausted quickly; `K_overgrow` is reached only after most meaningful paths are already in `paths_concurr`
- The window between loop exit and `wait_for_all` completion is short → few orphans generated
- Max error 1.7 still shows the bug exists, just rarely triggered

### Summary Pattern

```
Error magnitude ∝ (fan-out × in-flight tasks at loop exit × P(orphan ∈ top-K))
                ∝ (|E|/|V| × _num_workers × bulk_size × (K / total_paths))
```

`aes_core` has the worst ratio of `K` to total reachable paths — it's dense enough that the top-K boundary lies in a region with many equal-cost paths, making orphaned nodes much more likely to belong to the true top-K set.