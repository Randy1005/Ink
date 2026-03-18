#!/usr/bin/env python3
"""
Analysis framework for _spur_multiq optimization in Ink.

Pipeline (all three run, outputs cached between runs):
  Phase 1: Explorer + Analyzer  (parallel)
  Phase 2: Planner

Claude reads debug_session/03_plan.md and acts as Critic / Coder / Fixer.
"""

import asyncio
import os
from pathlib import Path

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------
INK_ROOT  = Path(__file__).parent.parent
INK_CPP   = INK_ROOT / "ink" / "ink.cpp"
INK_HPP   = INK_ROOT / "ink" / "ink.hpp"
OUT_DIR   = INK_ROOT / "debug_session"
BIG_TABLE = INK_ROOT / "big-table.csv"

# ---------------------------------------------------------------------------
# Provider config  — "anthropic" or "gemini"
# ---------------------------------------------------------------------------
PROVIDER = "gemini"

ANTHROPIC_FAST   = "claude-sonnet-4-6"
GEMINI_FAST      = "gemini-2.5-flash"


def fast_model() -> str:
    return ANTHROPIC_FAST if PROVIDER == "anthropic" else GEMINI_FAST


def _async_client():
    if PROVIDER == "anthropic":
        import anthropic
        return anthropic.AsyncAnthropic()
    else:
        import openai
        return openai.AsyncOpenAI(
            api_key=os.environ.get("GEMINI_API_KEY", ""),
            base_url="https://generativelanguage.googleapis.com/v1beta/openai/",
        )


# ---------------------------------------------------------------------------
# Code reader
# ---------------------------------------------------------------------------
def _read_lines(path: Path) -> list[str]:
    return path.read_text().splitlines()


def _slice(lines: list[str], start: int, end: int) -> str:
    return "\n".join(f"{start + i:4d}  {l}" for i, l in enumerate(lines[start - 1:end]))


def gather_code() -> dict[str, str]:
    cpp = _read_lines(INK_CPP)
    hpp = _read_lines(INK_HPP)
    return {
        "spur_multiq_toplevel":   _slice(cpp, 1865, 2090),
        "spur_tbb_task_vecs_ref": _slice(cpp, 2821, 2930),
        "spur_mlq_reference":     _slice(cpp, 1695, 1862),
        "pfxt_cache_multiq":      _slice(cpp, 1408, 1424),
        "push_task":              _slice(cpp, 3323, 3370),
        "determine_q_idx":        _slice(hpp,  510,  528),
        "set_num_task_qs":        _slice(hpp,  530,  560),
    }


def code_block(label: str, code: str) -> str:
    return f"### {label}\n```cpp\n{code}\n```\n"


def format_code_context(code: dict[str, str]) -> str:
    sections = [
        ("_spur_multiq top-level (ink.cpp:1865) — MODIFIABLE",
         code["spur_multiq_toplevel"]),
        ("_spur_tbb_task_vecs (ink.cpp:2821) — read-only reference",
         code["spur_tbb_task_vecs_ref"]),
        ("_spur_mlq / cpathgen reference (ink.cpp:1695) — read-only",
         code["spur_mlq_reference"]),
        ("_pfxt_cache_multiq (ink.cpp:1408) — read-only",
         code["pfxt_cache_multiq"]),
        ("push_task (ink.cpp:3323) — read-only",
         code["push_task"]),
        ("determine_q_idx (ink.hpp:510) — read-only",
         code["determine_q_idx"]),
        ("set_num_task_qs (ink.hpp:530) — read-only",
         code["set_num_task_qs"]),
    ]
    return "\n".join(code_block(label, c) for label, c in sections)


# ---------------------------------------------------------------------------
# Async streaming helper
# ---------------------------------------------------------------------------
async def stream_agent(system: str, user: str, label: str) -> str:
    print(f"[{label}] thinking...", flush=True)
    chunks: list[str] = []
    client = _async_client()

    if PROVIDER == "anthropic":
        async with client.messages.stream(
            model=fast_model(),
            max_tokens=16000,
            system=[{"type": "text", "text": system,
                     "cache_control": {"type": "ephemeral"}}],
            messages=[{"role": "user", "content": user}],
        ) as stream:
            async for event in stream:
                if (event.type == "content_block_delta"
                        and hasattr(event.delta, "text")):
                    chunks.append(event.delta.text)
    else:
        stream = await client.chat.completions.create(
            model=fast_model(),
            max_tokens=16000,
            messages=[
                {"role": "system", "content": system},
                {"role": "user",   "content": user},
            ],
            stream=True,
        )
        async for chunk in stream:
            if chunk.choices and chunk.choices[0].delta.content:
                chunks.append(chunk.choices[0].delta.content)

    result = "".join(chunks)
    print(f"[{label}] done ({len(result)} chars)", flush=True)
    return result


# ---------------------------------------------------------------------------
# Agent 1 — Explorer
# ---------------------------------------------------------------------------
EXPLORER_SYSTEM = """\
You are an expert C++ performance analyst focused on parallel algorithms.

Analyse the _spur_multiq top-level function in Ink's path-generation engine.
The function now uses TBB concurrent_vector + level+window loop (no Taskflow,
no moody camel queues). Focus on where time is actually spent and what
structural bottlenecks remain.

Report structure (use these exact headings):
1. Algorithm summary — what _spur_multiq does now (TBB level+window)
2. Hot path — which operations dominate wall time (allocation, sync, data movement)
3. Parallelism quality — how well work is distributed across threads
4. Comparison with cpathgen (_spur_mlq) — remaining structural differences
5. Suspected bottlenecks — ranked by likely impact, with line references
"""

async def run_explorer(code_context: str) -> str:
    report = await stream_agent(
        EXPLORER_SYSTEM,
        f"Source code to analyse:\n\n{code_context}",
        "EXPLORER",
    )
    (OUT_DIR / "01_explorer.md").write_text(report)
    print(f"[EXPLORER] saved → debug_session/01_explorer.md", flush=True)
    return report


# ---------------------------------------------------------------------------
# Agent 2 — Analyzer
# ---------------------------------------------------------------------------
ANALYZER_SYSTEM = """\
You are a performance profiling analyst for parallel C++ systems.

You will receive source code of _spur_multiq (TBB level+window implementation)
and benchmark timing results. The implementation is now CORRECT (0% error).
Focus purely on performance — where is time wasted and why.

Current known state:
- vga_lcd K=1M: pathgen ~165ms, ot (sequential baseline) ~157ms, cpathgen ~327ms
- drain_count = 0 on all benchmarks (no orphans)
- Main loop: tbb::parallel_for_each over concurrent_vector windows
- Per-node spur: _spur_tbb_task_vecs (allocates unique_ptr per child via make_unique)
- Overflow: remove_if + parallel redistribute when all active vecs drain

Report structure:
1. Allocation bottleneck — cost of make_unique<PfxtNode> per spur child
2. Concurrency bottleneck — push_back contention on tbb::concurrent_vector
3. Overflow compaction cost — remove_if scan on potentially large overflow vec
4. Memory access patterns — cache friendliness of PfxtNode, task_vecs layout
5. Benchmark correlation — which benchmarks are slowest and inferred why
6. Prioritised opportunities — top 3 optimizations ranked by expected gain / effort
"""

async def run_analyzer(code_context: str) -> str:
    benchmark = BIG_TABLE.read_text() if BIG_TABLE.exists() else "(big-table.csv not found)"
    user = f"## Source Code\n{code_context}\n\n## Benchmark Results\n```\n{benchmark}\n```"
    report = await stream_agent(ANALYZER_SYSTEM, user, "ANALYZER")
    (OUT_DIR / "02_analyzer.md").write_text(report)
    print(f"[ANALYZER] saved → debug_session/02_analyzer.md", flush=True)
    return report


# ---------------------------------------------------------------------------
# Agent 3 — Planner
# ---------------------------------------------------------------------------
PLANNER_SYSTEM = """\
You are a senior parallel-systems engineer proposing concrete optimizations.

The implementation is CORRECT and FAST but we want to push further.
Only Ink::_spur_multiq(size_t K, Pfxt&, bool, bool) in ink.cpp may be modified.
_spur_tbb_task_vecs is read-only but callable.

Current baseline (vga_lcd, K=1M): pathgen ~165ms vs ot ~157ms.
Target: meaningfully reduce pathgen_pfxt_time (≥5% gain counts).

For each proposed optimization:
1. Name and one-line description
2. Which lines/functions change
3. Expected mechanism of speedup (why it reduces time)
4. Estimated effort (S/M/L) and risk (low/med/high)
5. Concrete before/after pseudocode or diff sketch

Rank proposals by (expected gain × low risk / effort). Top proposal first.
Limit to 3 proposals — depth over breadth.

IMPORTANT CONSTRAINTS:
- Correctness is non-negotiable: pathgen_avg_err and pathgen_max_err must stay 0.0
- child cost >= parent cost (monotonicity) — exploit this aggressively for pruning
- No external dependencies beyond TBB, std, and existing Ink headers
"""

async def run_planner(code_context: str,
                      explorer_report: str,
                      analyzer_report: str) -> str:
    user = (f"## Explorer Report\n{explorer_report}\n\n"
            f"## Analyzer Report\n{analyzer_report}\n\n"
            f"## Source Code\n{code_context}")
    report = await stream_agent(PLANNER_SYSTEM, user, "PLANNER")
    (OUT_DIR / "03_plan.md").write_text(report)
    print(f"[PLANNER] saved → debug_session/03_plan.md", flush=True)
    return report


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
async def async_main() -> None:
    OUT_DIR.mkdir(exist_ok=True)

    print(f"[CONFIG] Provider={PROVIDER}  model={fast_model()}", flush=True)
    print("Gathering source code...", flush=True)
    code_context = format_code_context(gather_code())

    # Phase 1: Explorer + Analyzer in parallel (cached if files exist)
    f1, f2 = OUT_DIR / "01_explorer.md", OUT_DIR / "02_analyzer.md"
    if f1.exists() and f2.exists():
        print("\nPhase 1: loading cached Explorer + Analyzer...", flush=True)
        explorer_report = f1.read_text()
        analyzer_report = f2.read_text()
    else:
        print("\nPhase 1: Explorer + Analyzer (parallel)...", flush=True)
        explorer_report, analyzer_report = await asyncio.gather(
            run_explorer(code_context),
            run_analyzer(code_context),
        )

    # Phase 2: Planner (cached if file exists)
    f3 = OUT_DIR / "03_plan.md"
    if f3.exists():
        print("\nPhase 2: loading cached Planner report...", flush=True)
    else:
        print("\nPhase 2: Planner...", flush=True)
        await run_planner(code_context, explorer_report, analyzer_report)

    print("\n✓ Done. Read debug_session/03_plan.md for optimization proposals.", flush=True)
    print("  Claude will evaluate and implement the best direction.", flush=True)


def main() -> None:
    asyncio.run(async_main())


if __name__ == "__main__":
    main()
