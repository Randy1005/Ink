The current `_spur_multiq` implementation leverages TBB effectively, but with a reported `pathgen ~165ms` vs `ot ~157ms` on `vga_lcd` (K=1M), there's a slight slowdown compared to sequential `ot` for this specific benchmark, indicating potential parallel overheads or contention points. The provided `*.edges` benchmarks show good speedups over the sequential baseline, but further optimization can push performance for all scenarios.

A crucial constraint is that `_spur_tbb_task_vecs` is read-only. This means we cannot alter `std::make_unique<PfxtNode>` or the direct `task_vecs[vec_id].push_back` calls *within* that function. Our optimizations must focus on the `_spur_multiq` function itself, including how it prepares data for `_spur_tbb_task_vecs` and how it processes its output.

Here are three concrete optimization proposals, ranked by (expected gain × low risk / effort):

---

### Proposal 1: Double-Buffered Overflow Management

1.  **Name and one-line description:**
    **Double-Buffered Overflow Management:** Refactor the overflow promotion and compaction to use a new temporary `tbb::concurrent_vector` for remaining nodes and `tbb::combinable` for batching promoted nodes, reducing contention and avoiding expensive in-place `remove_if`/`resize`.

2.  **Which lines/functions change:**
    Lines 1952-2007 in `ink.cpp` (`_spur_multiq` function).

3.  **Expected mechanism of speedup (why it reduces time):**
    The existing overflow handling (lines 1983-2007) has two main performance issues:
    *   **Contention during Promotion:** The `parallel_for_each` loop (L1986-1997) directly calls `task_vecs[idx].push_back(std::move(pfx))` for each promoted node. If many threads promote nodes to the same active cost band (`task_vecs[idx]`), this leads to significant contention on that specific `tbb::concurrent_vector`'s internal locks or atomic operations.
    *   **Inefficient Compaction:** `std::remove_if` followed by `resize` (L2000-2006) on `task_vecs.back()` is costly. `std::remove_if` scans the entire overflow vector, and `resize` on `tbb::concurrent_vector` can involve memory reallocations and copies, especially for large vectors.

    This proposal addresses both by:
    *   **Batching Promotions:** Using `tbb::combinable` with thread-local buffers (a `std::vector` for each target `task_vecs[idx]`). Threads accumulate promoted nodes locally. After `parallel_for_each`, a `combine_each` loop bulk-inserts these buffered nodes into their respective `task_vecs[idx]` using `grow_by`. This significantly reduces the frequency and contention of `push_back` operations on the active `task_vecs`.
    *   **Efficient Compaction:** Simultaneously, non-promoted nodes are moved into a *new*, temporary `tbb::concurrent_vector`. After all promotions are processed, the old `task_vecs.back()` is cleared and populated from this `new_overflow_vec` (or by using `std::exchange` if `tbb::concurrent_vector` supported it directly). This avoids the full scan of `remove_if` and the `resize` overhead.

4.  **Estimated effort (S/M/L) and risk (low/med/high):**
    *   **Effort:** M (Moderate). Requires careful management of `std::unique_ptr` ownership and `tbb::combinable` with multiple target buffers.
    *   **Risk:** M (Medium). Correctness depends on meticulous handling of node transfers and ensuring no data loss.

5.  **Concrete before/after pseudocode or diff sketch:**

```diff
--- a/ink.cpp
+++ b/ink.cpp
@@ -1983,29 +1983,50 @@
       if (done) break;
   
       // Move promoted nodes from overflow into their correct active vecs
       arena.execute([&] {
+        // Thread-local storage for promoted nodes, grouped by target queue ID.
+        // Each thread gets a vector of vectors, where inner vectors correspond to target task_vecs.
+        tbb::combinable<std::vector<std::vector<std::unique_ptr<PfxtNode>>>> local_promoted_buffers([&]{
+            // Initialize each thread's local storage with 'num_task_qs - 1' empty vectors
+            return std::vector<std::vector<std::unique_ptr<PfxtNode>>>(num_task_qs - 1);
+        });
+
+        // A new concurrent_vector to collect nodes that remain in the overflow queue.
+        tbb::concurrent_vector<std::unique_ptr<PfxtNode>> new_overflow_vec;
+
         oneapi::tbb::parallel_for_each(
           task_vecs.back().begin(),
           task_vecs.back().end(),
           [&](auto& pfx) {
             if (!pfx) return;
             size_t idx = 0;
             for (; idx < num_task_qs - 1; idx++) {
               if (pfx->cost <= bounds[idx]) break;
             }
-            if (idx == num_task_qs - 1) return; // stays in overflow
-            task_vecs[idx].push_back(std::move(pfx));
+
+            if (idx == num_task_qs - 1) { // Node should stay in overflow
+              new_overflow_vec.push_back(std::move(pfx));
+            } else { // Node is promoted
+              local_promoted_buffers.local()[idx].push_back(std::move(pfx));
+            }
           });
+
+        // After all threads have processed, combine their local promoted buffers
+        local_promoted_buffers.combine_each([&](std::vector<std::vector<std::unique_ptr<PfxtNode>>>& thread_queues) {
+          for (size_t i = 0; i < num_task_qs - 1; ++i) { // Iterate over target active queues
+            if (!thread_queues[i].empty()) {
+              // Bulk-insert into the global task_vecs[i] using grow_by
+              auto it = task_vecs[i].grow_by(thread_queues[i].size());
+              std::move(thread_queues[i].begin(), thread_queues[i].end(), it);
+            }
+          }
+        });
       });
   
-      // Compact overflow: remove promoted (null) entries
-      auto rm_end = std::remove_if(
-        std::execution::par_unseq,
-        task_vecs.back().begin(),
-        task_vecs.back().end(),
-        [](auto& pfx) { return !pfx; });
-      task_vecs.back().resize(
-        std::distance(task_vecs.back().begin(), rm_end));
+      // Replace the old overflow vector with the newly built one (containing only non-promoted nodes)
+      task_vecs.back().clear(); // Clear existing elements
+      if (!new_overflow_vec.empty()) {
+        task_vecs.back().assign(std::make_move_iterator(new_overflow_vec.begin()), std::make_move_iterator(new_overflow_vec.end()));
+      }
     }
```

---

### Proposal 2: Streamlined Drain Phase

1.  **Name and one-line description:**
    **Streamlined Drain Phase:** Consolidate multiple collection and `std::nth_element` steps in the drain phase into a single pass to reduce data movement and computational overhead.

2.  **Which lines/functions change:**
    Lines 2009-2090 in `ink.cpp` (`_spur_multiq` function).

3.  **Expected mechanism of speedup (why it reduces time):**
    The current drain phase performs several potentially costly operations:
    *   Multiple allocations of `std::vector<std::unique_ptr<PfxtNode>>` (`result`, `orphans`).
    *   Multiple iterations to move `std::unique_ptr`s between these vectors.
    *   Multiple `std::nth_element` calls (L2021, L2038, L2088) on potentially large collections.
    *   Creation of a separate `std::vector<float> all_costs` (L2034) just for threshold calculation.

    This proposal aims to streamline this by:
    1.  **Single Collection:** Gather *all* candidate `PfxtNode`s (from `paths_cv` and all `task_vecs`) into a single `std::vector<std::unique_ptr<PfxtNode>>` at the start of the drain phase.
    2.  **Single Thresholding:** Perform `std::nth_element` once on this combined collection to identify the `K`-th shortest path's cost and establish the tightest possible `threshold`.
    3.  **Filtered Population:** Populate `drain_cv` (the `tbb::concurrent_vector` used for the iterative re-spurring) directly from this single, partitioned vector, including only nodes below or at the threshold.
    4.  **Reduced Finalization:** After the drain loop completes, `drain_cv` already contains all relevant paths. A single copy to a final `std::vector` and `std::nth_element` (if `drain_cv.size() > K`) will finalize the result.

    This reduces memory allocations, data copying, and expensive sorting operations, improving overall performance, especially when `K` is large or many "orphan" paths exist.

4.  **Estimated effort (S/M/L) and risk (low/med/high):**
    *   **Effort:** M (Moderate). Restructuring the flow of `unique_ptr`s requires careful implementation to maintain correctness.
    *   **Risk:** M (Medium). Correctness of the final `K` paths depends on the new logic accurately identifying and processing all paths within the threshold.

5.  **Concrete before/after pseudocode or diff sketch:**

```diff
--- a/ink.cpp
+++ b/ink.cpp
@@ -2009,47 +2009,40 @@
     // paths_cv has >= K entries if K was reached; unprocessed nodes remain in task_vecs.
     // Compute a tight threshold and drain all qualifying orphan nodes.
     if (path_cnt >= K) {
       auto by_cost = [](const auto& a, const auto& b) { return a->cost < b->cost; };
   
-      // Collect paths_cv -> result; trim to K; get initial threshold
-      std::vector<std::unique_ptr<PfxtNode>> result;
-      for (auto& p : paths_cv) if (p) result.push_back(std::move(p));
+      // Collect all current paths (from paths_cv and all task_vecs) into one std::vector
+      std::vector<std::unique_ptr<PfxtNode>> all_candidate_nodes;
+      all_candidate_nodes.reserve(paths_cv.size() + K); // Heuristic reservation
+
+      for (auto& p : paths_cv) if (p) all