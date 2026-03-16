# Technical Report: `_spur_multiq` Function Family

---

## 1. Purpose

`_spur_multiq` implements a parallel K-shortest-paths generator using a multi-level priority queue architecture. Its role is to expand a path-prefix tree (Pfxt) by iteratively "spurring" candidate nodes — each spur step enumerates all detour edges from a given prefix node's tail vertex along the suffix tree, producing child nodes that represent longer path prefixes. The K (or K × `overgrow_scalar`) shortest paths are accumulated in `pfxt.paths_concurr` / `_spurred_nodes`. The multi-queue structure (`task_qs[0..num_task_qs-1]`) partitions nodes by cost into priority bands so that lower-cost (higher-priority) nodes are processed before higher-cost ones, approximating a priority queue without global locking.

---

## 2. Top-level Control Flow

The outer function (`ink.cpp:1863`) operates as a **spin-submit loop** over a Taskflow executor:

```
_atom_path_cnt = 0
while (_atom_path_cnt < K_overgrow):
    executor.silent_async(lambda):
        1. scan task_qs[0..num_task_qs-1] for first non-empty queue → q_id
        2. if updating_bounds && spur_ahead:
               for id in [q_id, num_task_qs-2]:
                   executor.silent_async(lambda): pop 2 nodes, spur each, enqueue to paths_concurr
               return  (outer task returns without incrementing _atom_path_cnt)
        3. else if q_id == num_task_qs-1 && enable_node_redistr:
               CAS on updating_bounds; recompute bounds; CAS on redistributing;
               drain task_qs[last] → _tmp_q → redistribute to task_qs[determine_q_idx(cost)]
               return  (no path count increment)
        4. else if _bulk_size != 1:
               pop bulk of nodes from task_qs[q_id]; spur each; enqueue to paths_concurr
               _atom_path_cnt += sz
        5. else:
               pop single node from task_qs[q_id]; spur it
               _atom_path_cnt++
executor.wait_for_all()
_spurred_nodes = move(pfxt.paths_concurr)
```

Key characteristics:
- The main thread submits tasks **continuously** without waiting; it only stops submitting when `_atom_path_cnt >= K_overgrow`.
- `_atom_path_cnt` is a class-level `std::atomic` (implied by usage with `+=` from multiple lambdas), but the scan loop (`while pfxt.task_qs[q_id].size_approx() == 0`) is not atomic and races with concurrent enqueuers.
- `executor.wait_for_all()` at line 2014 is the only synchronization barrier before results are consumed.
- The loop can submit a large number of overlapping async tasks before any complete; there is no back-pressure mechanism.

---

## 3. Per-node Spur

The per-node overload (`ink.cpp:2711`, signature `_spur_multiq(Pfxt&, PfxtNode&, tf::Executor&)`) walks the suffix tree from `pfx.to` toward the sink `T`:

```
u = pfx.to
while u != sfxt.T:
    for each edge in fanout(u):
        for each weight selector w_sel:
            skip if no weight
            skip if dest v not in sfxt (no sfxt.dists[v])
            skip if edge == sfxt.links[u]  (suffix-tree edge, not a detour)
            
            detour_cost = sfxt.dists[v] + w - sfxt.dists[u]
            c  = detour_cost + pfx.cost
            dc = detour_cost + pfx.detour_cost
            pfxt.push_task(ink, c, dc, u, v, edge, &pfx, encode(edge, w_sel))
    
    u = sfxt.successors[u]   // advance along suffix path
return spur_cnt
```

`push_task` (line 3263) calls `determine_q_idx(c)` and enqueues a new `PfxtNode` into `task_qs[q_idx]`. The `tf::Executor&` parameter is accepted but **never used** inside this function body — it is vestigial or reserved.

The parent pointer `&pfx` is stored raw in the child node. The parent node's lifetime depends on it having been moved into `paths_concurr` *before or concurrently* with the child being popped and processed — there is no explicit lifetime guarantee enforced here.

---

## 4. Queue Routing — `determine_q_idx` + Bounds Interaction

`determine_q_idx` (ink.hpp:524) performs a linear scan over `bounds[0..N-2]` (where N = `num_task_qs`):

```
i = 0
while i < bounds.size():
    if i == 0: break if c < bounds[0]
    else:      break if bounds[i-1] <= c < bounds[i]
    i++
return i   // returns N-1 (last queue) if c >= all bounds
```

The boundary semantics differ between `determine_q_idx` and `_spur_tbb_task_vecs`:
- `determine_q_idx`: uses `is_in_bounds(c, bounds[i-1], bounds[i])` (definition not shown; assumed half-open interval).
- `_spur_tbb_task_vecs`: uses `c < bounds[vec_id]` (strict less-than on lower bound only).

Bounds are initialised in `set_num_task_qs` from `old_min_dc` / `old_max_dc`:
- **EQUAL policy**: `bounds[i] = old_min_dc + _width * (i+1)`, uniform spacing.
- **GEOMETRIC policy**: `bounds[i] = old_min_dc + base^(i+1)`, exponential spacing.

During redistribution (lines 1949–1976), bounds are **shifted upward** relative to `bounds.back()` (the last boundary value, not last queue), not relative to `old_min_dc`. The redistribution loop has a logic bug: `while (!_tmp_q.size_approx() == 0)` parses as `while ((!_tmp_q.size_approx()) == 0)`, i.e., `while (0 == 0)` — an **infinite loop** unless the `!` is intended as `!=`. The `size_approx() == 0` check is also non-atomic with concurrent producers.

The `bounds` vector is a shared mutable object accessed from concurrent lambdas. The `updating_bounds` atomic flag provides a partial CAS guard, but there is a window between `updating_bounds = false` (line 1960) and `redistributing = false` (line 1979) where another task can re-enter the bounds-update path and observe stale bounds while redistribution is still in progress.

---

## 5. Termination

The outer `while` loop terminates when `_atom_path_cnt >= K_overgrow`. However, several issues affect this termination condition:

**Counter increments occur only in paths 4 and 5** (bulk pop and single pop). Paths 2 (`spur_ahead` branch) and 3 (redistribution branch) both `return` without incrementing `_atom_path_cnt`, meaning the loop can spin indefinitely submitting redistribution or `spur_ahead` tasks while the counter stagnates.

**Queue exhaustion is not handled.** If all task queues become genuinely empty (all spur possibilities exhausted), the inner scan exits at `q_id == num_task_qs - 1`, triggers redistribution (if enabled), and loops — never detecting true emptiness. The comment at line 1899–1901 acknowledges this as a known gap: *"TODO: how would we know if the N queues are 'really' empty."*

**`_atom_path_cnt` is read without a fence** in the loop condition by the main thread while being written by worker threads via `+=`. Taskflow's `silent_async` does not guarantee that increments are visible to the submitting thread before `wait_for_all`.

**`executor.wait_for_all()` at line 2014** ensures all already-submitted tasks complete before `_spurred_nodes` is populated, but tasks submitted in the last iteration of the while loop may themselves submit further `silent_async` children (the `spur_ahead` branch submits nested tasks), which are also awaited by `wait_for_all`.

---

## 6. Key Data Flows

**`pfxt.paths_concurr`** (a concurrent queue):
- Populated by `pop_task(q_id)` single variant (line 3287-3288): ownership is transferred from `task_qs[q_id]` into `paths_concurr`, and a raw observer pointer is returned to the caller.
- In the bulk variant (`pop_task(q_id, bulk_size)`, line 3296): ownership is returned to the caller in a vector; the caller then explicitly calls `pfxt.paths_concurr.enqueue(std::move(n))` (lines 1999, 2928). There is an **asymmetry**: the single-pop path transfers to `paths_concurr` inside `pop_task`, while the bulk-pop path does so in the lambda.
- In the `spur_ahead` branch (line 1928): nodes are enqueued to `paths_concurr` explicitly after spurring.

**`_spurred_nodes`**:
- Receives the entire `paths_concurr` queue via move at line 2019, after `wait_for_all`.
- This is the final output of the function — a concurrent queue of all processed `PfxtNode` unique_ptrs.

**`pfxt.task_qs[i]`** (array of lock-free concurrent queues):
- Source of work items; drained by `pop_task`.
- Replenished by `push_task` called from `_spur_multiq(pfxt, node, executor)`.
- Subject to move-drain during redistribution (`_tmp_q = std::move(pfxt.task_qs[q_id])`), which is unsafe if other threads are concurrently accessing `task_qs[q_id]` — `redistributing` CAS only serialises redistributors against each other, not against concurrent poppers.

**`PfxtNode::parent` raw pointer**:
- Set to `&pfx` in `push_task`. The pointed-to node lives in `paths_concurr` after being dequeued and moved there. Because `paths_concurr` holds `unique_ptr`s that are only moved (not destroyed) until `_spurred_nodes` is consumed, the raw pointer remains valid for the duration of the algorithm, assuming no node is destroyed prematurely.

---

## 7. Comparison with `_spur_mlq` Reference — Structural Differences

| Dimension | `_spur_multiq` | `_spur_mlq` (reference) |
|---|---|---|
| **Parallelism runtime** | Taskflow (`tf::Executor`, `silent_async`) | Intel TBB (`tbb::task_arena`, `parallel_for_each`) |
| **Task container** | Lock-free MPMC queues (`task_qs[]`, moodycamel) | TBB concurrent vectors (`tbb::concurrent_vector`) |
| **Scheduling model** | Continuous submission loop; tasks submitted by main thread each iteration | Bulk wave-front: all nodes in queue `id` are processed before advancing to `id+1` |
| **Termination criterion** | Atomic counter `_atom_path_cnt >= K_overgrow` checked on main thread | Explicit `path_cnt >= K` check with `done` flag inside deterministic wave loop |
| **Bounds update** | CAS-guarded async update; non-deterministic timing relative to processing | Synchronous: bounds updated between waves, with a `while (num_promoted == 0)` guarantee |
| **Redistribution** | In-place async drain of last queue into `_tmp_q`, then redistribute; unsynchronized against concurrent poppers | Serial reassignment within `parallel_for_each` over last vec; safe because wave processing has completed |
| **`spur_ahead` mode** | Distinct code path that nests additional `silent_async` tasks and skips counter increment | Not present |
| **Output storage** | `paths_concurr` (concurrent queue) → `_spurred_nodes` | `_tbb_cv_paths` (TBB concurrent vector) |
| **Parent lifetime** | Raw pointer into `paths_concurr`; relies on queue persistence | Raw pointer into `_tbb_cv_paths`; same reliance |
| **Queue exhaustion** | Unhandled (acknowledged TODO) | Handled implicitly: if last vec is empty and no nodes promoted, `num_promoted` stays 0 and delta keeps growing until a node qualifies — though this too can loop if all nodes are truly exhausted |
| **Per-node spur signature** | `_spur_multiq(Pfxt&, PfxtNode&, tf::Executor&)` — executor unused | `_spur_tbb_task_vecs(Pfxt&, PfxtNode&, vector<concurrent_vector>&)` — task_vecs actively used for routing |
| **Child routing** | `push_task` → `determine_q_idx` (linear scan on `bounds`) | Inline linear scan `c < bounds[vec_id]` directly in spur function |
| **Overgrow factor** | `K * overgrow_scalar` (configurable scalar) | Exact K |
| **Step/progress tracking** | `_total_workloads`, `pfxt_time` | `num_steps`, `accum_path_cnt_per_step`, `pfxt_time` |

The most structurally significant difference is the **scheduling discipline**: `_spur_mlq` processes all nodes in priority band `id` to completion before moving to band `id+1` (wave-front), ensuring that the K cheapest paths are found in order. `_spur_multiq` operates as a **continuous free-running submission loop** where tasks from any band may execute concurrently and out of order, relying on the overgrow scalar and post-hoc sorting (if any) to recover the K shortest paths. This makes `_spur_multiq` less deterministic in path ordering but potentially higher throughput under heavy parallelism.