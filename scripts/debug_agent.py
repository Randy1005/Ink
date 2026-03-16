#!/usr/bin/env python3
"""
Multi-agent debugging framework for _spur_multiq in Ink.

Pipeline:
  Phase 1: Explorer + Analyzer (parallel)
  Phase 2: Planner
  Phase 3: Critic  (interactive review — may ask you questions)
  Phase 4: Fix loop (up to MAX_FIX_ATTEMPTS accuracy failures):
    - Fixer applies the plan to ink.cpp
    - Build  (compilation errors auto-retried, don't count as attempts)
    - Verify (run big-table, check pathgen accuracy)
    - Success → done
    - Failure → re-plan with error feedback
    - MAX_FIX_ATTEMPTS exhausted → Guidance session (asks you for insights),
      re-plan, reset counter, repeat

Only _spur_multiq (both overloads) may be modified.
"""

import asyncio
import csv as csv_module
import json
import os
import re
import subprocess
import sys
from pathlib import Path

import anthropic

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------
INK_ROOT   = Path(__file__).parent.parent
INK_CPP    = INK_ROOT / "ink" / "ink.cpp"
INK_HPP    = INK_ROOT / "ink" / "ink.hpp"
OUT_DIR    = INK_ROOT / "debug_session"
BIG_TABLE  = INK_ROOT / "big-table.csv"

# ---------------------------------------------------------------------------
# Provider config  — set PROVIDER to "anthropic" or "gemini"
# ---------------------------------------------------------------------------
PROVIDER = "gemini"   # ← change this to switch providers

# Anthropic models
ANTHROPIC_FAST   = "claude-sonnet-4-6"
ANTHROPIC_STRONG = "claude-opus-4-6"

# Gemini models (via OpenAI-compatible API)
GEMINI_FAST   = "gemini-3.1-flash-lite-preview"
GEMINI_STRONG = "gemini-3.1-pro-preview"

# Keep these as aliases so existing code that uses SONNET/OPUS still compiles
SONNET = ANTHROPIC_FAST
OPUS   = ANTHROPIC_STRONG


def fast_model() -> str:
    return ANTHROPIC_FAST if PROVIDER == "anthropic" else GEMINI_FAST


def strong_model() -> str:
    return ANTHROPIC_STRONG if PROVIDER == "anthropic" else GEMINI_STRONG


def _anthropic_async() -> anthropic.AsyncAnthropic:
    return anthropic.AsyncAnthropic()


def _anthropic_sync() -> anthropic.Anthropic:
    return anthropic.Anthropic()


def _gemini_async():
    import openai
    return openai.AsyncOpenAI(
        api_key=os.environ.get("GEMINI_API_KEY", ""),
        base_url="https://generativelanguage.googleapis.com/v1beta/openai/",
    )


def _gemini_sync():
    import openai
    return openai.OpenAI(
        api_key=os.environ.get("GEMINI_API_KEY", ""),
        base_url="https://generativelanguage.googleapis.com/v1beta/openai/",
    )


MAX_FIX_ATTEMPTS = 5   # accuracy failures before asking user for guidance
MAX_BUILD_RETRIES = 3  # compilation failures before giving up
ERROR_THRESHOLD   = 0.1  # percent — below this counts as "zero error"
BENCHMARKS = ["tv80", "aes_core", "des_perf", "vga_lcd"]
K_PATHS    = 20000


# ---------------------------------------------------------------------------
# Code reader
# ---------------------------------------------------------------------------
def _read_file_lines(path: Path) -> list[str]:
    return path.read_text().splitlines()


def _slice_lines(lines: list[str], start: int, end: int) -> str:
    chunk = lines[start - 1 : end]
    return "\n".join(f"{start + i:4d}  {l}" for i, l in enumerate(chunk))


def gather_code() -> dict[str, str]:
    cpp_lines = _read_file_lines(INK_CPP)
    hpp_lines = _read_file_lines(INK_HPP)
    return {
        "spur_multiq_toplevel":    _slice_lines(cpp_lines, 1863, 2020),
        "spur_multiq_pernode":     _slice_lines(cpp_lines, 2711, 2749),
        "pfxt_cache_multiq":       _slice_lines(cpp_lines, 1411, 1427),
        "push_task":               _slice_lines(cpp_lines, 3254, 3306),
        "spur_mlq_reference":      _slice_lines(cpp_lines, 1698, 1859),
        "spur_tbb_task_vecs_ref":  _slice_lines(cpp_lines, 2752, 2805),
        "determine_q_idx":         _slice_lines(hpp_lines,  524,  541),
        "set_num_task_qs":         _slice_lines(hpp_lines,  544,  563),
    }


def code_block(label: str, code: str) -> str:
    return f"### {label}\n```cpp\n{code}\n```\n"


def format_fixer_context(code: dict[str, str]) -> str:
    """Slim context for the Fixer — only the two modifiable functions."""
    sections = [
        ("_spur_multiq top-level (ink.cpp:1863) — MODIFY THIS",
         code["spur_multiq_toplevel"]),
        ("_spur_multiq per-node (ink.cpp:2711) — MODIFY THIS",
         code["spur_multiq_pernode"]),
    ]
    return "\n".join(code_block(label, c) for label, c in sections)


def format_code_context(code: dict[str, str]) -> str:
    sections = [
        ("_spur_multiq top-level (ink.cpp:1863) — TARGET FOR MODIFICATION",
         code["spur_multiq_toplevel"]),
        ("_spur_multiq per-node (ink.cpp:2711) — TARGET FOR MODIFICATION",
         code["spur_multiq_pernode"]),
        ("_pfxt_cache_multiq (ink.cpp:1411) — read-only",
         code["pfxt_cache_multiq"]),
        ("push_task / pop_task (ink.cpp:3254) — read-only",
         code["push_task"]),
        ("determine_q_idx (ink.hpp:524) — read-only",
         code["determine_q_idx"]),
        ("set_num_task_qs (ink.hpp:544) — read-only",
         code["set_num_task_qs"]),
        ("_spur_mlq (cpathgen reference, ink.cpp:1698) — read-only",
         code["spur_mlq_reference"]),
        ("_spur_tbb_task_vecs (cpathgen reference, ink.cpp:2752) — read-only",
         code["spur_tbb_task_vecs_ref"]),
    ]
    return "\n".join(code_block(label, c) for label, c in sections)


# ---------------------------------------------------------------------------
# Async streaming helper  (Explorer / Analyzer / Planner)
# ---------------------------------------------------------------------------
async def stream_agent(model: str,
                       system: str,
                       user: str,
                       label: str,
                       thinking: bool = False,
                       timeout_secs: int | None = None) -> str:
    print(f"[{label}] thinking...", flush=True)
    chunks: list[str] = []

    if PROVIDER == "anthropic":
        client = _anthropic_async()
        params: dict = dict(
            model=model,
            max_tokens=16000,
            system=[{"type": "text", "text": system,
                     "cache_control": {"type": "ephemeral"}}],
            messages=[{"role": "user", "content": user}],
        )
        if thinking:
            params["thinking"] = {"type": "adaptive"}

        async def _do_stream() -> None:
            async with client.messages.stream(**params) as stream:
                async for event in stream:
                    if (event.type == "content_block_delta"
                            and hasattr(event.delta, "text")):
                        chunks.append(event.delta.text)
    else:
        client = _gemini_async()

        async def _do_stream() -> None:
            stream = await client.chat.completions.create(
                model=model,
                max_tokens=16000,
                messages=[
                    {"role": "system", "content": system},
                    {"role": "user", "content": user},
                ],
                stream=True,
            )
            async for chunk in stream:
                if chunk.choices and chunk.choices[0].delta.content:
                    chunks.append(chunk.choices[0].delta.content)

    timed_out = False
    try:
        if timeout_secs is not None:
            await asyncio.wait_for(_do_stream(), timeout=timeout_secs)
        else:
            await _do_stream()
    except asyncio.TimeoutError:
        timed_out = True

    result = "".join(chunks)
    if timed_out:
        print(f"[{label}] timeout ({timeout_secs}s) — using partial output ({len(result)} chars)", flush=True)
    else:
        print(f"[{label}] done ({len(result)} chars)", flush=True)
    return result


# ---------------------------------------------------------------------------
# Shared interactive tool loop  (Critic + Guidance reuse this)
# ---------------------------------------------------------------------------
ASK_USER_TOOL = {
    "name": "ask_user",
    "description": (
        "Ask the developer a targeted question to clarify domain knowledge "
        "needed to validate, reject, or improve the proposed fix."
    ),
    "input_schema": {
        "type": "object",
        "properties": {
            "question": {"type": "string", "description": "The specific question to ask."}
        },
        "required": ["question"],
    },
}


def _interactive_tool_loop(system: str,
                            user_content: str,
                            label: str,
                            out_file: Path) -> str:
    interactive = sys.stdin.isatty()
    mode = "will ask you questions" if interactive else "non-interactive, auto-answering"
    print(f"[{label}] thinking... ({mode})", flush=True)

    final_text = ""

    if PROVIDER == "anthropic":
        client = _anthropic_sync()
        messages: list[dict] = [{"role": "user", "content": user_content}]
        try:
            while True:
                response = client.beta.messages.create(
                    model=fast_model(),
                    max_tokens=16000,
                    system=[{"type": "text", "text": system,
                             "cache_control": {"type": "ephemeral"}}],
                    tools=[ASK_USER_TOOL],
                    messages=messages,
                    betas=["compact-2026-01-12"],
                    context_management={"edits": [{"type": "compact_20260112"}]},
                )
                text_parts: list[str] = []
                for block in response.content:
                    if block.type == "text":
                        print(block.text)
                        text_parts.append(block.text)
                if response.stop_reason == "end_turn":
                    final_text = "\n".join(text_parts)
                    break
                tool_results = []
                for block in response.content:
                    if block.type == "tool_use" and block.name == "ask_user":
                        question = block.input.get("question", "")
                        print(f"\n[{label} asks]: {question}")
                        answer = input("Your answer: ").strip() if interactive else (
                            "No additional context available; proceed with your best assessment.")
                        if not interactive:
                            print(f"[auto]: {answer}")
                        tool_results.append({
                            "type": "tool_result",
                            "tool_use_id": block.id,
                            "content": answer,
                        })
                messages.append({"role": "assistant", "content": response.content})
                if tool_results:
                    messages.append({"role": "user", "content": tool_results})
                else:
                    break
        finally:
            if final_text:
                out_file.write_text(final_text)
                print(f"[{label}] saved → {out_file.relative_to(INK_ROOT)}", flush=True)

    else:  # gemini — OpenAI-compatible function calling
        client = _gemini_sync()
        ASK_USER_TOOL_OAI = {
            "type": "function",
            "function": {
                "name": "ask_user",
                "description": ASK_USER_TOOL["description"],
                "parameters": ASK_USER_TOOL["input_schema"],
            },
        }
        messages_oai: list[dict] = [
            {"role": "system", "content": system},
            {"role": "user", "content": user_content},
        ]
        try:
            while True:
                response = client.chat.completions.create(
                    model=fast_model(),
                    max_tokens=16000,
                    messages=messages_oai,
                    tools=[ASK_USER_TOOL_OAI],
                    tool_choice="auto",
                )
                choice = response.choices[0]
                msg = choice.message
                if msg.content:
                    print(msg.content)
                    final_text = msg.content
                if choice.finish_reason == "stop":
                    break
                tool_results_oai: list[dict] = []
                if msg.tool_calls:
                    messages_oai.append({
                        "role": "assistant",
                        "content": msg.content or "",
                        "tool_calls": [tc.model_dump() for tc in msg.tool_calls],
                    })
                    for tc in msg.tool_calls:
                        if tc.function.name == "ask_user":
                            args = json.loads(tc.function.arguments)
                            question = args.get("question", "")
                            print(f"\n[{label} asks]: {question}")
                            answer = input("Your answer: ").strip() if interactive else (
                                "No additional context available; proceed with your best assessment.")
                            if not interactive:
                                print(f"[auto]: {answer}")
                            tool_results_oai.append({
                                "role": "tool",
                                "tool_call_id": tc.id,
                                "content": answer,
                            })
                    messages_oai.extend(tool_results_oai)
                else:
                    break
        finally:
            if final_text:
                out_file.write_text(final_text)
                print(f"[{label}] saved → {out_file.relative_to(INK_ROOT)}", flush=True)

    return final_text


# ---------------------------------------------------------------------------
# Agent 1 — Explorer
# ---------------------------------------------------------------------------
EXPLORER_SYSTEM = """\
You are an expert C++ static analyser focused on parallel algorithms.

Your only job is to produce a concise, accurate technical report on the
_spur_multiq function family in Ink's path-generation engine.

CONSTRAINT: You are NOT asked to suggest fixes. Only analyse and report.

Report structure (use these exact headings):
1. Purpose — what _spur_multiq is supposed to do
2. Top-level control flow — how the Taskflow executor loop works
3. Per-node spur — how children are generated and queued
4. Queue routing — how determine_q_idx + bounds interact
5. Termination — how/when the outer while loop stops
6. Key data flows — what ends up in paths_concurr / _spurred_nodes
7. Comparison with cpathgen reference (_spur_mlq) — structural differences
"""

async def run_explorer(code_context: str) -> str:
    report = await stream_agent(
        fast_model(), EXPLORER_SYSTEM,
        f"Here is the source code to analyse:\n\n{code_context}",
        "EXPLORER",
    )
    (OUT_DIR / "01_explorer.md").write_text(report)
    return report


# ---------------------------------------------------------------------------
# Agent 2 — Analyzer  (runs in parallel with Explorer)
# ---------------------------------------------------------------------------
ANALYZER_SYSTEM = """\
You are a concurrency bug analyst specialising in lock-free multi-queue systems.

You will receive source code of _spur_multiq (both overloads), benchmark error
measurements, and a key developer insight about the primary error mechanism.

PRIMARY INSIGHT (treat as confirmed):
  The main source of path cost error is EARLY TERMINATION.
  When _atom_path_cnt reaches K_overgrow, the outer while loop stops submitting
  new silent_async tasks. executor.wait_for_all() waits for already-submitted
  tasks, but those tasks push children into pfxt.task_qs via push_task — and
  these children are NEVER picked up after the loop exits. Some orphaned children
  belong to the true top-K set → missing from paths_concurr → path cost errors.

CONSTRAINT: Only _spur_multiq (both overloads) may be modified.

Report structure:
1. Primary root cause — early termination (exact chain)
2. Secondary issues — any additional bugs inside _spur_multiq only
3. Evidence map — lines involved, why each causes error
4. Benchmark correlation — which benchmarks show most error and why
"""

async def run_analyzer(code_context: str) -> str:
    benchmark = BIG_TABLE.read_text() if BIG_TABLE.exists() else "(big-table.csv not found)"
    user = f"## Source Code\n{code_context}\n\n## Benchmark Results\n```\n{benchmark}\n```"
    report = await stream_agent(fast_model(), ANALYZER_SYSTEM, user, "ANALYZER")
    (OUT_DIR / "02_analyzer.md").write_text(report)
    return report


# ---------------------------------------------------------------------------
# Agent 3 — Planner  (also used for retry with error feedback)
# ---------------------------------------------------------------------------
PLANNER_BASE = """\
You are a senior parallel-systems engineer designing a targeted bug fix.

STRICT CONSTRAINT: You may ONLY modify code inside:
  - Ink::_spur_multiq(size_t K, Pfxt&, bool, bool)    [ink.cpp:1863]
  - Ink::_spur_multiq(Pfxt&, PfxtNode&, tf::Executor&) [ink.cpp:2711]
Do NOT propose changes to any other function.

DEVELOPER INSIGHT: K_overgrow is NOT provably correct. Goal: THEORETICALLY EXACT
algorithm without relying on any overgrow scalar.

THRESHOLD-BASED DRAIN (provably exact):
  1. Run parallel phase until _atom_path_cnt >= K (drop overgrow_scalar).
  2. After wait_for_all(), sort paths_concurr; threshold = cost of K-th path.
  3. Drain ALL task_qs: spur nodes with cost <= threshold; discard the rest.
  4. Re-sort paths_concurr; recompute threshold from new K-th path.
  5. Repeat 3-4 until no queued node has cost <= threshold.
  6. Return top-K from paths_concurr.
  WHY EXACT: after the drain loop, remaining queued nodes all have cost > threshold.
  Costs are monotonically non-decreasing along spur chains, so no descendant can
  have cost <= threshold. QED.

Produce a concrete fix plan — be concise:
1. Chosen approach and rationale (2-3 sentences)
2. Exact before/after code (unified diff style) for both overloads
3. Why the change closes the early-termination gap (1-2 sentences)
4. Edge cases (bullet list only)
"""

PLANNER_SYSTEM = PLANNER_BASE

PLANNER_RETRY_SYSTEM = PLANNER_BASE + """
RETRY CONTEXT: A previous fix was applied but benchmark errors persist.
Study the error measurements carefully. Identify what went wrong and revise.
Do NOT repeat analysis already in the Explorer/Analyzer reports.
"""

async def run_planner(code_context: str,
                      explorer_report: str,
                      analyzer_report: str,
                      extra_context: str = "") -> str:
    user = (f"## Explorer Report\n{explorer_report}\n\n"
            f"## Analyzer Report\n{analyzer_report}\n\n"
            f"## Source Code\n{code_context}")
    if extra_context:
        user += f"\n\n{extra_context}"
    report = await stream_agent(fast_model(), PLANNER_SYSTEM, user, "PLANNER", timeout_secs=120)
    (OUT_DIR / "03_plan.md").write_text(report)
    return report


async def run_planner_retry(code_context: str,
                            explorer_report: str,
                            analyzer_report: str,
                            prev_plan: str,
                            error_results: str) -> str:
    user = (f"## Previous Plan\n{prev_plan}\n\n"
            f"## Benchmark Errors After Previous Fix\n```\n{error_results}\n```\n\n"
            f"## Explorer Report\n{explorer_report}\n\n"
            f"## Analyzer Report\n{analyzer_report}\n\n"
            f"## Current Source Code\n{code_context}")
    report = await stream_agent(fast_model(), PLANNER_RETRY_SYSTEM, user, "PLANNER-RETRY", timeout_secs=120)
    (OUT_DIR / "03_plan.md").write_text(report)
    return report


# ---------------------------------------------------------------------------
# Agent 4 — Critic  (interactive review of the initial plan)
# ---------------------------------------------------------------------------
CRITIC_SYSTEM = """\
You are a critical reviewer of parallel algorithm fixes.

You will receive Explorer/Analyzer/Planner reports and the full source context.

DEVELOPER REQUIREMENT: The fix must be PROVABLY EXACT — equivalent to the
sequential golden — without relying on the K_overgrow heuristic.

Your job:
1. Verify the proposed fix provides a formal exactness guarantee. Can any path
   in the true top-K be missing from paths_concurr after the fix? Prove it cannot,
   or show a counterexample.
2. Challenge the threshold-drain loop:
   - Is the termination condition tight enough?
   - Does the drain introduce new races?
   - Is "cost monotonically non-decreasing" valid for this graph?
3. Ask the developer targeted questions via ask_user for domain knowledge.
4. Produce a FINAL VERDICT: approve, reject, or revise — with a precise
   revised diff if needed.

CONSTRAINT: Only _spur_multiq (both overloads) may be modified.
Be sceptical but constructive.

IMPORTANT: At the end of your review you MUST output your FINAL VERDICT as plain
text (not via ask_user). This text is saved and passed to the Fixer — without it
the fix loop cannot start. Format it as:

## FINAL VERDICT
[approve / reject / revise]

## Key Issues Found
[bullet list]

## Revised Instructions for Fixer
[precise instructions incorporating developer answers and your analysis]
"""

def run_critic(code_context: str,
               explorer_report: str,
               analyzer_report: str,
               planner_report: str) -> str:
    user_content = (
        f"## Explorer Report\n{explorer_report}\n\n"
        f"## Analyzer Report\n{analyzer_report}\n\n"
        f"## Planner Report (proposed fix)\n{planner_report}\n\n"
        f"## Source Code\n{code_context}"
    )
    return _interactive_tool_loop(
        CRITIC_SYSTEM, user_content, "CRITIC",
        OUT_DIR / "04_critique.md",
    )


# ---------------------------------------------------------------------------
# Agent 5 — Fixer  (writes new code into ink.cpp)
# ---------------------------------------------------------------------------
FIXER_SYSTEM = """\
You are a C++ implementer applying a targeted fix to the Ink path-generation engine.

You will receive:
- CURRENT source of both _spur_multiq overloads
- The approved fix plan (and Critic's verdict if present)
- Optionally: a BUILD ERROR from a previous attempt

OUTPUT FORMAT — use these exact delimiters on their own lines:
<<<TOPLEVEL_START>>>
<complete new implementation of void Ink::_spur_multiq(size_t K, Pfxt&, bool, bool)>
<<<TOPLEVEL_END>>>
<<<PERNODE_START>>>
<complete new implementation of size_t Ink::_spur_multiq(Pfxt&, PfxtNode&, tf::Executor&)>
<<<PERNODE_END>>>

RULES:
- Output ONLY the two function bodies between delimiters — nothing else
- Do NOT truncate — every line must be present and syntactically complete
- Preserve all existing helper calls and member variable names unless the plan
  explicitly changes them
- If fixing a build error, address only the specific error; stay true to the plan
"""

async def run_fixer(current_toplevel: str,
                    current_pernode: str,
                    plan: str,
                    critic_report: str = "",
                    build_error: str = "") -> tuple[str | None, str | None]:
    user = (f"## Current _spur_multiq Top-Level\n```cpp\n{current_toplevel}\n```\n\n"
            f"## Current _spur_multiq Per-Node\n```cpp\n{current_pernode}\n```\n\n"
            f"## Fix Plan\n{plan}")
    if critic_report:
        user += f"\n\n## Critic's Verdict / Revision\n{critic_report}"
    if build_error:
        user += f"\n\n## Build Error to Fix\n```\n{build_error[-3000:]}\n```"

    print("[FIXER] generating fix...", flush=True)
    if PROVIDER == "anthropic":
        client = _anthropic_async()
        response = await client.messages.create(
            model=fast_model(),
            max_tokens=16000,
            system=[{"type": "text", "text": FIXER_SYSTEM,
                     "cache_control": {"type": "ephemeral"}}],
            messages=[{"role": "user", "content": user}],
        )
        text = next((b.text for b in response.content if b.type == "text"), "")
    else:
        client = _gemini_async()
        response = await client.chat.completions.create(
            model=fast_model(),
            max_tokens=16000,
            messages=[
                {"role": "system", "content": FIXER_SYSTEM},
                {"role": "user", "content": user},
            ],
        )
        text = response.choices[0].message.content or ""

    m_top = re.search(r'<<<TOPLEVEL_START>>>\n(.*?)<<<TOPLEVEL_END>>>', text, re.DOTALL)
    m_per = re.search(r'<<<PERNODE_START>>>\n(.*?)<<<PERNODE_END>>>', text, re.DOTALL)

    if not m_top or not m_per:
        print("[FIXER] ERROR: missing output delimiters in response", flush=True)
        print(f"[FIXER] raw snippet: {text[:300]}", flush=True)
        return None, None

    print("[FIXER] done", flush=True)
    return m_top.group(1).strip(), m_per.group(1).strip()


def _find_function_end(lines: list[str], start: int) -> int:
    depth = 0
    for i in range(start, len(lines)):
        for ch in lines[i]:
            if ch == '{':
                depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0 and i > start:
                    return i
    return len(lines) - 1


def _find_spur_multiq_ranges(lines: list[str]) -> tuple[tuple[int, int], tuple[int, int]]:
    """Returns ((toplevel_start, end), (pernode_start, end)) as 0-based indices."""
    toplevel_start = pernode_start = None
    for i, line in enumerate(lines):
        if 'void Ink::_spur_multiq(' in line and toplevel_start is None:
            toplevel_start = i
        elif 'size_t Ink::_spur_multiq(' in line and pernode_start is None:
            pernode_start = i
    if toplevel_start is None or pernode_start is None:
        raise ValueError("Could not locate _spur_multiq functions in ink.cpp")
    return (
        (toplevel_start, _find_function_end(lines, toplevel_start)),
        (pernode_start,  _find_function_end(lines, pernode_start)),
    )


def apply_fix(toplevel_code: str, pernode_code: str) -> None:
    """Replace both _spur_multiq implementations in ink.cpp in-place."""
    lines = INK_CPP.read_text().splitlines()
    (ts, te), (ps, pe) = _find_spur_multiq_ranges(lines)
    # Replace pernode first (higher index) so toplevel indices remain valid
    lines[ps:pe + 1] = pernode_code.splitlines()
    lines[ts:te + 1] = toplevel_code.splitlines()
    INK_CPP.write_text('\n'.join(lines) + '\n')
    print(f"[FIXER] ink.cpp patched (toplevel ~L{ts+1}, pernode ~L{ps+1})", flush=True)


# ---------------------------------------------------------------------------
# Build & Verify
# ---------------------------------------------------------------------------
def build_project() -> tuple[bool, str]:
    print("[BUILD] compiling...", flush=True)
    result = subprocess.run(
        ['cmake', '--build', str(INK_ROOT / 'build'), '--parallel'],
        capture_output=True, text=True,
    )
    ok = result.returncode == 0
    output = result.stdout + result.stderr
    print(f"[BUILD] {'success' if ok else 'FAILED'}", flush=True)
    if not ok:
        print(output[-1500:], flush=True)
    return ok, output


def _kill_proc(proc) -> None:
    """Kill process group reliably; fall back to plain kill."""
    import os, signal
    try:
        os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
    except Exception:
        try:
            proc.kill()
        except Exception:
            pass
    # Don't call proc.wait() here — let caller decide


def run_verifier(big_table_bin: Path) -> dict[str, dict]:
    """Run each benchmark. Type 'hang<Enter>' in the terminal to mark the
    current benchmark as an infinite-loop and kill it immediately."""
    import threading

    if BIG_TABLE.exists():
        BIG_TABLE.unlink()

    BENCHMARK_TIMEOUT = 30  # safety net; user can type 'hang' to abort sooner
    timed_out: list[str] = []
    print("[VERIFY] running benchmarks... (type 'hang<Enter>' to abort current)", flush=True)

    for bm in BENCHMARKS:
        edges  = INK_ROOT / 'benchmarks' / f'{bm}.edges'
        golden = INK_ROOT / 'golden'     / f'{bm}.golden'
        if not edges.exists() or not golden.exists():
            continue
        print(f"[VERIFY]   {bm}...", end=" ", flush=True)

        proc = subprocess.Popen(
            [str(big_table_bin), str(K_PATHS), str(edges), str(golden)],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
            start_new_session=True,
        )

        hang_flag = threading.Event()

        def _stdin_watcher(flag: threading.Event, p) -> None:
            """Background thread: read stdin; if user types 'hang', set flag and kill."""
            try:
                line = sys.stdin.readline().strip().lower()
                if line == "hang":
                    flag.set()
                    _kill_proc(p)
            except Exception:
                pass

        watcher = threading.Thread(target=_stdin_watcher, args=(hang_flag, proc), daemon=True)
        watcher.start()

        hung = False
        try:
            proc.wait(timeout=BENCHMARK_TIMEOUT)
            if hang_flag.is_set():
                hung = True
        except subprocess.TimeoutExpired:
            _kill_proc(proc)
            hung = True
            print(f"AUTO-TIMEOUT (>{BENCHMARK_TIMEOUT}s)", end=" ", flush=True)

        if hung or hang_flag.is_set():
            timed_out.append(bm)
            print("HANG — aborting verify, all remaining benchmarks assumed hung", flush=True)
            break  # no point running the rest
        else:
            print("done", flush=True)

    results: dict[str, dict] = {}
    if BIG_TABLE.exists():
        with open(BIG_TABLE) as f:
            for row in csv_module.DictReader(f):
                results[row['benchmark']] = {
                    'pathgen_avg_err': float(row.get('pathgen_avg_err', 0)),
                    'pathgen_max_err': float(row.get('pathgen_max_err', 0)),
                }
    # Mark all benchmarks that didn't finish (hung + any skipped after first hang)
    ran = set(results.keys())
    for bm in BENCHMARKS:
        if bm in timed_out or (timed_out and bm not in ran):
            results[bm] = {'pathgen_avg_err': float('inf'), 'pathgen_max_err': float('inf'),
                           'timed_out': True}
    return results


def _errors_resolved(results: dict[str, dict]) -> bool:
    return bool(results) and all(
        v['pathgen_avg_err'] < ERROR_THRESHOLD and v['pathgen_max_err'] < ERROR_THRESHOLD
        for v in results.values()
    )


def _format_results(results: dict[str, dict]) -> str:
    if not results:
        return "(no results)"
    lines = ["benchmark             avg_err%  max_err%"]
    for bm, v in sorted(results.items()):
        if v.get('timed_out'):
            lines.append(f"  {bm:<20}  TIMEOUT — infinite loop suspected")
        else:
            lines.append(f"  {bm:<20} {v['pathgen_avg_err']:8.3f}  {v['pathgen_max_err']:8.3f}")
    return '\n'.join(lines)


def _format_history(history: list[dict]) -> str:
    return '\n\n'.join(
        f"### Attempt {h['attempt']}\n"
        f"**Plan excerpt:** {h['plan_summary']}\n\n"
        f"**Results:**\n```\n{h['results']}\n```"
        for h in history
    )


# ---------------------------------------------------------------------------
# Agent 6 — Guidance  (Critic-style: summarises failures, asks user for help)
# ---------------------------------------------------------------------------
GUIDANCE_SYSTEM = f"""\
You are a senior engineer reviewing a stalled fix cycle for the Ink path-generation
engine. The fix team has made {MAX_FIX_ATTEMPTS} attempt(s) but pathgen accuracy
errors persist.

You will receive:
- A history of all fix attempts with their benchmark error measurements
- The current approved plan and current code state

Your job:
1. Briefly summarise what was tried and why it likely failed
2. Ask the developer targeted questions via ask_user to gather new domain insights
3. Based on answers, produce a concise set of REVISED INSTRUCTIONS for the next
   planning cycle (this output goes directly to the Planner as extra context)

Be direct. Focus questions on root causes not yet understood.
"""

def run_guidance(attempt_history: str,
                 current_code: str,
                 current_plan: str) -> str:
    user_content = (
        f"## Fix Attempt History\n{attempt_history}\n\n"
        f"## Current Plan\n{current_plan}\n\n"
        f"## Current Code State\n{current_code}"
    )
    return _interactive_tool_loop(
        GUIDANCE_SYSTEM, user_content, "GUIDANCE",
        OUT_DIR / "guidance.md",
    )


# ---------------------------------------------------------------------------
# Phase 4 — Fix loop
# ---------------------------------------------------------------------------
async def fix_loop(
    explorer_report: str,
    analyzer_report: str,
    planner_report: str,
    critic_report: str,
    big_table_bin: Path,
) -> None:
    current_plan = planner_report
    attempt = 0
    attempt_history: list[dict] = []

    while True:
        attempt += 1
        print(f"\n=== Fix Attempt {attempt}/{MAX_FIX_ATTEMPTS} ===", flush=True)

        # Read current function bodies (slim — only modifiable functions)
        cpp_lines = _read_file_lines(INK_CPP)
        (ts, te), (ps, pe) = _find_spur_multiq_ranges(cpp_lines)
        current_toplevel = '\n'.join(cpp_lines[ts:te + 1])
        current_pernode  = '\n'.join(cpp_lines[ps:pe + 1])
        code_context = format_code_context(gather_code())  # full context for re-planner

        # Save a backup of ink.cpp before touching it so we can always revert
        ink_backup = INK_CPP.read_text()

        def revert_ink() -> None:
            INK_CPP.write_text(ink_backup)
            print("[REVERT] ink.cpp restored to pre-patch state", flush=True)

        # Apply fix — retry on build errors (doesn't count as an attempt)
        build_error = ""
        build_ok = False
        for build_try in range(MAX_BUILD_RETRIES + 1):
            if build_try == MAX_BUILD_RETRIES:
                revert_ink()
                # Summarise what happened and ask user how to proceed
                print("\n[BUILD] All retries exhausted.", flush=True)
                print(f"[BUILD] Last error (tail):\n{build_error[-1500:]}", flush=True)
                print("\n[BUILD] Likely cause: fixer generated syntactically invalid C++ "
                      "(missing braces, wrong types, undefined symbols, etc.).", flush=True)
                print("\nOptions:", flush=True)
                print("  c  — continue: re-plan and try again", flush=True)
                print("  q  — quit", flush=True)
                choice = input("Your choice [c/q]: ").strip().lower()
                if choice != "c":
                    print("Aborting.", flush=True)
                    return
                # Count this as an accuracy attempt so we don't loop forever
                attempt_history.append({
                    'attempt': attempt,
                    'plan_summary': current_plan[:600] + ('...' if len(current_plan) > 600 else ''),
                    'results': f"BUILD FAILED after {MAX_BUILD_RETRIES} retries:\n{build_error[-800:]}",
                })
                break  # skip verify, go straight to re-plan

            toplevel_code, pernode_code = await run_fixer(
                current_toplevel, current_pernode,
                current_plan,
                critic_report if attempt == 1 else "",
                build_error,
            )
            if toplevel_code is None:
                revert_ink()
                print("[ERROR] Fixer produced no valid output — giving up.", flush=True)
                return

            apply_fix(toplevel_code, pernode_code)
            ok, build_out = build_project()
            if ok:
                build_ok = True
                break
            build_error = build_out
            print(f"[BUILD] retry {build_try + 1}/{MAX_BUILD_RETRIES}...", flush=True)

        if build_ok:
            # Verify accuracy
            results = run_verifier(big_table_bin)
            result_str = _format_results(results)
            print(f"[VERIFY]\n{result_str}", flush=True)

            attempt_history.append({
                'attempt': attempt,
                'plan_summary': current_plan[:600] + ('...' if len(current_plan) > 600 else ''),
                'results': result_str,
            })

            if _errors_resolved(results):
                print(f"\n✓ Pathgen errors below {ERROR_THRESHOLD}% after {attempt} attempt(s)!", flush=True)
                (OUT_DIR / "05_fix_result.md").write_text(
                    f"# Fix Successful — Attempt {attempt}\n\n## Results\n```\n{result_str}\n```"
                )
                return

        # Re-plan or ask for guidance
        code_context = format_code_context(gather_code())

        if attempt < MAX_FIX_ATTEMPTS:
            print(f"[LOOP] errors remain — re-planning...", flush=True)
            current_plan = await run_planner_retry(
                code_context, explorer_report, analyzer_report,
                current_plan, result_str,
            )
        else:
            print(f"\n[LOOP] {MAX_FIX_ATTEMPTS} attempts exhausted — requesting your guidance...",
                  flush=True)
            new_insights = run_guidance(
                _format_history(attempt_history), code_context, current_plan,
            )
            current_plan = await run_planner(
                code_context, explorer_report, analyzer_report,
                extra_context=(f"## User Guidance After {MAX_FIX_ATTEMPTS} Failed Attempts\n"
                               f"{new_insights}"),
            )
            attempt = 0  # reset counter for next round


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
async def async_main() -> None:
    OUT_DIR.mkdir(exist_ok=True)

    big_table_bin = INK_ROOT / 'examples' / 'cpathgen' / 'big-table'
    if not big_table_bin.exists():
        print(f"[VERIFY] big-table binary not found at default path: {big_table_bin}", flush=True)
        answer = input("Enter path to big-table binary (or Enter to abort): ").strip()
        if not answer:
            print("[ERROR] No binary provided — aborting.", flush=True)
            return
        big_table_bin = Path(answer)
        if not big_table_bin.exists():
            print(f"[ERROR] Not found: {big_table_bin} — aborting.", flush=True)
            return
        print(f"[VERIFY] using binary: {big_table_bin}", flush=True)

    print(f"[CONFIG] Provider: {PROVIDER} (fast={fast_model()}, strong={strong_model()})", flush=True)
    print("Gathering source code…", flush=True)
    code_context = format_code_context(gather_code())

    # --- Phase 1 ---
    f1 = OUT_DIR / "01_explorer.md"
    f2 = OUT_DIR / "02_analyzer.md"
    if f1.exists() and f2.exists():
        print("\nPhase 1: loading cached Explorer + Analyzer reports…", flush=True)
        explorer_report  = f1.read_text()
        analyzer_report  = f2.read_text()
    else:
        print("\nPhase 1: Explorer + Analyzer (parallel)…", flush=True)
        explorer_report, analyzer_report = await asyncio.gather(
            run_explorer(code_context),
            run_analyzer(code_context),
        )

    # --- Phase 2 ---
    f3 = OUT_DIR / "03_plan.md"
    if f3.exists():
        print("\nPhase 2: loading cached Planner report…", flush=True)
        planner_report = f3.read_text()
    else:
        print("\nPhase 2: Planner…", flush=True)
        planner_report = await run_planner(code_context, explorer_report, analyzer_report)

    # --- Phase 3 ---
    f4 = OUT_DIR / "04_critique.md"
    if f4.exists():
        print("\nPhase 3: loading cached Critic report…", flush=True)
        critic_report = f4.read_text()
    else:
        print("\nPhase 3: Critic review…", flush=True)
        critic_report = run_critic(code_context, explorer_report, analyzer_report, planner_report)

    print("\nPhase 4: Fix loop…", flush=True)
    await fix_loop(explorer_report, analyzer_report, planner_report, critic_report, big_table_bin)

    print("\n✓ Done. Reports in debug_session/", flush=True)


def main() -> None:
    asyncio.run(async_main())


if __name__ == "__main__":
    main()
