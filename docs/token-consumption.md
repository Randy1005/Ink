# Token Consumption Analysis

**Metric: output tokens** (model-generated tokens per optimization pass and total session)

---

## Comparison

| Approach             | System                                        | Output tokens / pass | Total (session) | Passes / runs |
|----------------------|-----------------------------------------------|----------------------|-----------------|---------------|
| Multi-agent pipeline | Gemini 2.5 Flash (debug_agent.py)             | ~10,964              | ~43,900         | 4 runs        |
| Conversational agent | Claude Sonnet 4.6 (Claude Code + superpowers) | ~105,000             | 1,049,309       | 10 passes     |

---

## Per-Agent Breakdown (Gemini, one run)

| Agent     | Input tokens | Output tokens |
|-----------|--------------|---------------|
| Explorer  | 10,136       | 4,848         |
| Analyzer  | 10,555       | 3,003         |
| Planner   | 18,119       | 3,113         |
| **Total** | **38,810**   | **10,964**    |

---

## Methodology Notes

- **Output tokens** = model-generated tokens only; independent of user prompt length or context reuse patterns.
- **Claude Code totals** are exact, measured from Claude Code session JSONL files (`~/.claude/projects/*/**.jsonl`).
- **Gemini totals** are exact per run, measured via `stream_options={"include_usage": True}` added to `scripts/debug_agent.py`.
- **Passes**: 10 total (iter3–6 for pathgen + cpathgen c1–c6). **Runs**: 4 debug_agent.py invocations across the project.
- The two approaches serve different scopes: Gemini generates proposals only; Claude Code evaluates, implements, debugs, and verifies. They are measured on the same per-pass basis for comparability.
- Claude Code uses prompt caching extensively; cache-read tokens (~296M) are not counted as output tokens and are billed at ~10× lower rate.

---

## Appendix A: Example Output — Gemini Multi-Agent Pipeline (Proposal 1 of 3)

> One proposal from a single `debug_agent.py` run (Explorer → Analyzer → Planner).
> This is the full content of `debug_session/03_plan.md`, Proposal 1.

---

### Parallelize Overflow Node Collection

**One-line description:**
Collect non-promoted nodes into thread-local buffers and perform a bulk merge, eliminating contention on the shared `new_overflow` `tbb::concurrent_vector`.

**Which lines/functions change:**

- `_spur_multiq` lines 2036–2037: Modify the type definition of `promo_bufs`.
- `_spur_multiq` line 2038: Replace the direct definition of `new_overflow` with a global collection point.
- `_spur_multiq` lines 2050–2051: Change how non-promoted nodes are collected within the `parallel_for_each`.
- `_spur_multiq` lines 2059–2076: Modify the `promo_bufs.combine_each` lambda to handle bulk flushing of both promoted and non-promoted nodes.

**Expected mechanism of speedup:**
Currently, nodes that remain in the overflow queue after promotion are directly `push_back`ed into `new_overflow` (`tbb::concurrent_vector`) by multiple threads concurrently (line 2051). While `tbb::concurrent_vector::push_back` is thread-safe, it involves internal atomic operations and potentially contention for segment allocation or the vector's end pointer. By leveraging `tbb::combinable`, threads will instead write non-promoted nodes into their own local `std::vector`s. At the end of the parallel region, a single `combine_each` operation will efficiently bulk-merge these thread-local buffers into the global `tbb::concurrent_vector` using `grow_by` and `std::move`. This significantly reduces fine-grained contention during the high-volume collection phase of overflow redistribution.

**Effort:** Medium — **Risk:** Low

---

## Appendix B: Example Output — Claude Code + Superpowers (iter6 Implementation Plan)

> The implementation plan produced by the superpowers `writing-plans` skill for iter6.
> Claude Code follows this plan task-by-task using the `executing-plans` or `subagent-driven-development` skill.

---

### Flush Scan Reduction + Queue Count Tuning (iter6)

**Goal:** Eliminate the T=6 regression from iter5 and improve 4T performance by (A) starting the `combine_each` flush scan at `id` instead of 0, and (B) reducing `num_task_qs` from 100 to the smallest correct value.

**Architecture:** Approach A exploits cost monotonicity (child cost ≥ parent cost → bands 0..id-1 are provably empty during band-id processing) for a one-line change. Approach B reduces worst-case scan width at early bands, compounding the savings. Both are independent and can be reverted independently.

**File map:**

| File                                  | Change                                               |
|---------------------------------------|------------------------------------------------------|
| `ink/ink.cpp:1947–1955`               | Change `qi = 0` → `qi = id` in `combine_each` flush  |
| `examples/cpathgen/big-table.cpp:86`  | Sweep `num_queues` argument (100 → candidate Q)      |

**Task 2, Step 2 — Apply Approach A (excerpt):**

Change from:

```cpp
// Children may land in any band (0..num_task_qs-1), including already-processed
// or not-yet-reached bands — the flush correctly covers all qi.
tl_task_vecs.combine_each([&](auto& local_tv) {
  for (size_t qi = 0; qi < num_task_qs; qi++) {
```

to:

```cpp
// Children may land in bands >= id (cost monotonicity: child cost >= parent cost).
// Bands 0..id-1 are provably empty — scan starts at id.
tl_task_vecs.combine_each([&](auto& local_tv) {
  for (size_t qi = id; qi < num_task_qs; qi++) {
```

**Correctness gate (after every change):**

```bash
rm -f big-table.csv
for bm in vga_lcd leon2 leon3mp netcard; do
  examples/cpathgen/big-table 1000000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
# pathgen_avg_err and pathgen_max_err must be 0.0 on all rows
```

**Revert paths (independent per task):**

```bash
# Revert A only:
git checkout iter5-tl-task-vecs -- ink/ink.cpp

# Revert B only:
git checkout iter6-id-start -- examples/cpathgen/big-table.cpp

# Revert both:
git checkout iter5-tl-task-vecs -- ink/ink.cpp examples/cpathgen/big-table.cpp
```
