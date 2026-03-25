# Token Consumption Analysis

**Metric: output tokens per optimization pass**
(one complete "what should we do next?" cycle)

---

## Comparable: Proposal / Planning Phase

| Role | System | Output tokens / pass | What it does |
|------|--------|---------------------|--------------|
| Explorer + Analyzer + Planner | Gemini 2.5 Flash | **10,964** | Re-reads codebase + benchmark data fresh each run; generates ranked proposals |
| Critic (plan evaluation) | Claude Sonnet 4.6 | **~1,300** | Reads Gemini's plan, evaluates proposals, picks direction |

Both answer: *"what should we implement next?"*

---

## Not Comparable: Implementation Phase (Claude only)

| Role | System | Output tokens / pass | What it does |
|------|--------|---------------------|--------------|
| Coder + Fixer + Verifier | Claude Sonnet 4.6 | **~104,000** | Writes code, debugs build failures, runs benchmarks, iterates to correctness |
| — | Gemini | — | Not performed |

---

## Full Project Totals

| System | Output tokens | Passes / runs |
|--------|--------------|---------------|
| debug_agent.py (Gemini 2.5 Flash) | ~43,900 | 4 runs |
| Claude Code (all phases) | 1,049,309 | 10 passes |

---

## Methodology Notes

- **Output tokens** = model-generated tokens only; independent of user prompt length or context reuse patterns.
- **Claude Code totals** are exact, measured from Claude Code session JSONL files (`~/.claude/projects/*/**.jsonl`).
- **Gemini totals** are exact per run, measured via `stream_options={"include_usage": True}` added to `scripts/debug_agent.py`.
- **Claude critic estimate** (~1,300 tokens/pass) is derived by identifying plan-evaluation turns in the JSONL via keyword matching on output text.
- **Passes**: 10 total (iter3–6 for pathgen + cpathgen c1–c6). **Runs**: 4 debug_agent.py invocations across the project.
- Claude Code uses prompt caching extensively; cache-read tokens (~296M) are not counted as output tokens and are billed at ~10× lower rate.
