# Token Consumption Analysis

**Metric: output tokens per optimization pass**
(one complete cycle: analysis → proposal → implementation → verification)

---

## Per-Pass Comparison

| Role | System | Output tokens / pass | What it does |
|------|--------|---------------------|--------------|
| Explorer + Analyzer + Planner | Gemini 2.5 Flash (debug_agent.py) | **10,964** | Re-reads codebase + benchmark data fresh each run; generates ranked proposals |
| Critic | Claude Sonnet 4.6 (Claude Code + superpowers) | **~105,000** | Evaluates plan, picks direction, writes code, debugs build failures, verifies correctness |

The two roles form a pipeline: Gemini proposes, Claude executes.

---

## Per-Agent Breakdown (Gemini side, one run)

| Agent     | Input tokens | Output tokens |
|-----------|--------------|---------------|
| Explorer  | 10,136       | 4,848         |
| Analyzer  | 10,555       | 3,003         |
| Planner   | 18,119       | 3,113         |
| **Total** | **38,810**   | **10,964**    |

---

## Full Project Totals

| System | Output tokens | Passes / runs |
|--------|--------------|---------------|
| debug_agent.py (Gemini 2.5 Flash) | ~43,900 | 4 runs |
| Claude Code + superpowers (Critic) | 1,049,309 | 10 passes |

---

## Methodology Notes

- **Output tokens** = model-generated tokens only; independent of user prompt length or context reuse patterns.
- **Claude Code totals** are exact, measured from Claude Code session JSONL files (`~/.claude/projects/*/**.jsonl`).
- **Gemini totals** are exact per run, measured via `stream_options={"include_usage": True}` added to `scripts/debug_agent.py`.
- **Passes**: 10 total (iter3–6 for pathgen + cpathgen c1–c6). **Runs**: 4 debug_agent.py invocations across the project.
- Claude Code uses prompt caching extensively; cache-read tokens (~296M) are not counted as output tokens and are billed at ~10× lower rate.
