Here are three concrete optimization proposals for `Ink::_spur_multiq`, strictly adhering to the specified constraints. These proposals are ranked by their estimated impact, considering the "meaningfully reduce pathgen_pfxt_time (≥5% gain counts)" target, while prioritizing low risk and manageable effort.

---

### Proposal 1: Parallelize Overflow Node Collection

**1. Name and one-line description:**
Parallelize Overflow Node Collection: Collect non-promoted nodes into thread-local buffers and perform a bulk merge, eliminating contention on the shared `new_overflow` `tbb::concurrent_vector`.

**2. Which lines/functions change:**
*   `_spur_multiq` lines 2036-2037: Modify the type definition of `promo_bufs`.
*   `_spur_multiq` line 2038: Replace the direct definition of `new_overflow` with a global collection point.
*   `_spur_multiq` lines 2050-2051: Change how non-promoted nodes are collected within the `parallel_for_each`.
*   `_spur_multiq` lines 2059-2076: Modify the `promo_bufs.combine_each` lambda to handle bulk flushing of both promoted and non-promoted nodes.

**3. Expected mechanism of speedup:**
Currently, nodes that remain in the overflow queue after promotion are directly `push_back`ed into `new_overflow` (`tbb::concurrent_vector`) by multiple threads concurrently (line 2051). While `tbb::concurrent_vector::push_back` is thread-safe, it involves internal atomic operations and potentially contention for segment allocation or the vector's end pointer. By leveraging `tbb::combinable`, threads will instead write non-promoted nodes into their own local `std::vector`s. At the end of the parallel region, a single `combine_each` operation will efficiently bulk-merge these thread-local buffers into the global `tbb::concurrent_vector` (`global_new_overflow`) using `grow_by` and `std::move`. This significantly reduces fine-grained contention during the high-volume collection phase of overflow redistribution, leading to improved scalability and reduced synchronization overhead.

**4. Estimated effort (S/M/L) and risk (low/med/high):**
*   **Effort:** Medium. Requires changing a template type and adjusting the logic of a `combine_each` lambda, but it's a well-established TBB pattern.
*   **Risk:** Low. Uses standard TBB mechanisms for contention reduction and bulk data transfer. Correctness is maintained by `std::move` for ownership transfer and `grow_by` for thread-safe bulk insertion.

**5. Concrete before/after pseudocode or diff sketch:**

```diff
--- a/ink.cpp
+++ b/ink.cpp
@@ -2033,10 +2033,11 @@
       // Move promoted nodes from overflow into active vecs using thread-local
       // buffers (avoids per-element contention on task_vecs[idx].push_back).
       // Nodes that remain in overflow are collected into new_overflow.
-      tbb::combinable<std::vector<std::vector<std::unique_ptr<PfxtNode>>>> promo_bufs(
-        [&]{ return std::vector<std::vector<std::unique_ptr<PfxtNode>>>(num_task_qs - 1); });
-      tbb::concurrent_vector<std::unique_ptr<PfxtNode>> new_overflow;
+      tbb::combinable<std::pair<std::vector<std::vector<std::unique_ptr<PfxtNode>>>, std::vector<std::unique_ptr<PfxtNode>>>> promo_bufs(
+        [&]{ return std::make_pair(std::vector<std::vector<std::unique_ptr<PfxtNode>>>(num_task_qs - 1), std::vector<std::unique_ptr<PfxtNode>>()); });
+      // global_new_overflow will be populated in bulk via promo_bufs.combine_each
+      tbb::concurrent_vector<std::unique_ptr<PfxtNode>> global_new_overflow;
   
       arena.execute([&] {
         oneapi::tbb::parallel_for_each(
@@ -2048,26 +2049,36 @@
               if (pfx->cost <= bounds[idx]) break;
             }
             if (idx == num_task_qs - 1) {
-              new_overflow.push_back(std::move(pfx));
+              promo_bufs.local().second.push_back(std::move(pfx)); // Store in thread-local new_overflow buffer
             } else {
-              promo_bufs.local()[idx].push_back(std::move(pfx));
+              promo_bufs.local().first[idx].push_back(std::move(pfx));
             }
           });
       });
   
       // Flush thread-local promotion buffers into target active vecs (bulk grow_by)
-      promo_bufs.combine_each([&](std::vector<std::vector<std::unique_ptr<PfxtNode>>>& bufs) {
-        for (size_t i = 0; i < num_task_qs - 1; ++i) {
-          if (!bufs[i].empty()) {
-            auto it = task_vecs[i].grow_by(bufs[i].size());
-            std::move(bufs[i].begin(), bufs[i].end(), it);
-            bufs[i].clear();
+      promo_bufs.combine_each([&](std::pair<std::vector<std::vector<std::unique_ptr<PfxtNode>>>, std::vector<std::unique_ptr<PfxtNode>>>& bufs_pair) {
+        // Process promoted nodes
+        auto& bufs = bufs_pair.first;
+        for (size_t i = 0; i < num_task_qs - 1; ++i) {
+          if (!bufs[i].empty()) {
+            auto it = task_vecs[i].grow_by(bufs[i].size());
+            std::move(bufs[i].begin(), bufs[i].end(), it);
+            bufs[i].clear();
+          }
+        }
+        // Process non-promoted nodes (thread-local new_overflow)
+        auto& local_new_overflow = bufs_pair.second;
+        if (!local_new_overflow.empty()) {
+          auto it = global_new_overflow.grow_by(local_new_overflow.size());
+          std::move(local_new_overflow.begin(), local_new_overflow.end(), it);
+          local_new_overflow.clear();
+        }
+      });
+  
+      // Replace overflow with nodes that weren't promoted (no remove_if scan needed)
+      task_vecs.back().clear();
+      if (!global_new_overflow.empty()) {
+        auto it = task_vecs.back().grow_by(global_new_overflow.size());
+        std::move(global_new_overflow.begin(), global_new_overflow.end(), it);
+        global_new_overflow.clear(); // Clear for next iteration
+      }
+    }
+  
+    // --- Window drain ---
+    // paths_cv has >= K entries if K was reached; unprocessed nodes remain in task_vecs.
```

---

### Proposal 2: Parallel `min_element` for Equal Cost Band Overflow

**1. Name and one-line description:**
Parallel `min_element` for Equal Cost Band Overflow: Replace the serial `std::min_element` scan with a parallel version (`std::execution::par_unseq`) when determining the minimum cost in the overflow queue for the `EQUAL` partition policy.

**2. Which lines/functions change:**
*   `_spur_multiq` lines 1991-1997.

**3. Expected mechanism of speedup:**
The `PartitionPolicy::EQUAL` strategy involves finding the minimum cost node in `task_vecs.back()` (the overflow queue) using `std::min_element`. This is a serial `O(N)` operation, where `N` is the number of nodes in the overflow queue. If the overflow queue grows very large (e.g., millions of nodes), this single serial step can become a significant bottleneck, especially on multi-core systems where other work could be parallelized. By using `std::execution::par_unseq`, the `min_element` operation will be distributed across available threads, leveraging parallelism to drastically reduce its execution time, thus improving overall scalability of the overflow promotion phase.

**4. Estimated effort (S/M/L) and risk (low/med/high):**
*   **Effort:** Low. It's a single line modification to add the parallel execution policy.
*   **Risk:** Low. `std::execution::par_unseq` is a standard C++ feature for parallel algorithms, and its use is already demonstrated in the existing code (e.g., line 2022).

**5. Concrete before/after pseudocode or diff sketch:**

```diff
--- a/ink.cpp
+++ b/ink.cpp
@@ -1988,9 +1988,10 @@
   
       if (policy == PartitionPolicy::EQUAL) {
         // Compute minimum cost in overflow (single O(N) scan)
-        auto min_it = std::min_element(
-          task_vecs.back().begin(), task_vecs.back().end(),
-          [](const auto& a, const auto& b) {
+        auto min_it = std::min_element( 
+          std::execution::par_unseq, // Use parallel unsequenced execution
+          task_vecs.back().begin(), task_vecs.back().end(), 
+          [](const auto& a, const auto& b) { 
             if (!a) return false;
             if (!b) return true;
             return a->cost < b->cost;
```

---

### Proposal 3: Opportunistic `tbb::concurrent_vector::reserve` for Active Task Queues

**1. Name and one-line description:**
Opportunistic `tbb::concurrent_vector::reserve`: Pre-allocate memory for the active `task_vecs` and the overflow queue to reduce dynamic memory allocation overhead during `grow_by` operations.

**2. Which lines/functions change:**
*   `_spur_multiq` around line 1911 (after initial seeding, before the main expansion loop).

**3. Expected mechanism of speedup:**
`tbb::concurrent_vector` grows by allocating segments of memory. Frequent `grow_by` calls, especially if a segment fills up, can incur memory allocation overhead (e.g., calling `new`/`malloc`) and internal synchronization to manage new segments. By using `reserve` to pre-allocate a reasonable amount of memory, we reduce the likelihood of these dynamic segment allocations during the core `combine_each` operations (lines 1945, 1960, 2062). This amortizes allocation costs, can improve memory locality for the `std::unique_ptr`s themselves, and reduces potential contention on the memory allocator. The chosen `reserve` sizes are heuristics based on `K` and `num_task_qs`, aiming for a balance between reducing allocations and not wasting too much memory.

**4. Estimated effort (S/M/L) and risk (low/med/high):**
*   **Effort:** Low. Involves adding a few lines of `reserve` calls.
*   **Risk:** Low. `reserve` only affects capacity, not correctness. Over-reserving might slightly increase memory footprint but doesn't cause errors. Under-reserving just means the vector will grow dynamically as before, failing to achieve the full potential gain.

**5. Concrete before/after pseudocode or diff sketch:**

```diff
--- a/ink.cpp
+++ b/ink.cpp
@@ -1908,6 +1908,17 @@
     tbb::concurrent_vector<std::unique_ptr<PfxtNode>> paths_cv;
     paths_cv.reserve(K); // pre-allocate segments to avoid incremental growth during grow_by
     size_t path_cnt = 0;
+
+    // Opportunistic pre-reservation for task_vecs: reduce dynamic allocations during grow_by.
+    // Heuristic: Allocate a fraction of K for active bands, and a larger fraction for overflow.
+    // These values may need tuning based on specific workloads.
+    const size_t estimated_active_band_capacity = std::max(10000UL, K / (num_task_qs - 1) / 4);
+    const size_t estimated_overflow_capacity = std::max(100000UL, K / 2);
+    for (size_t qi = 0; qi < num_task_qs - 1; ++qi) {
+      task_vecs[qi].reserve(task_vecs[qi].size() + estimated_active_band_capacity);
+    }
+    task_vecs.back().reserve(task_vecs.back().size() + estimated_overflow_capacity);
+
     bool done = false;
   
     // --- Main expansion loop (cpathgen-style level + window) ---
```