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
