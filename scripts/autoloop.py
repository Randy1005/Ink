#!/usr/bin/env python3
"""
Autonomous delta optimization loop for cpathgen (_spur_mlq).

Each invocation:
  1. Loads autoloop_config.json
  2. Generates the config queue on first run
  3. Iterates through all remaining configs, running big-table for each
  4. Records results, updates best, appends to progress_log.md
  5. Commits and pushes changes

Config queue is organized in two phases:
  Phase 1 — static delta sweep (understand the landscape)
  Phase 2 — adaptive policy sweep (find best f(pps) -> delta rule)
"""

import csv
import json
import os
import re
import subprocess
import sys
from itertools import product
from pathlib import Path

# ── paths ─────────────────────────────────────────────────────────────────────
REPO        = Path(__file__).parent.parent
CONFIG_FILE = REPO / "scripts" / "autoloop_config.json"
PROGRESS_LOG = REPO / "progress_log.md"
BIG_TABLE   = REPO / "examples" / "cpathgen" / "big-table"
BENCHMARKS  = ["leon2", "leon3mp", "netcard"]
K           = 1_000_000
NUM_THREADS = 16
NUM_QUEUES  = 10

# A Phase 2 adaptive rule "wins" on a benchmark if its time is within this
# fraction of the per-benchmark oracle (best Phase 1 static delta result).
# 1.10 = within 10% of the best possible fixed-delta runtime.
WIN_THRESHOLD = 1.10

# ── config queue generation ────────────────────────────────────────────────────
def generate_queue():
    """Return the full list of configs to test, in order."""
    configs = []
    cid = 1

    # Phase 1: static delta sweep (target_pps=0 disables adaptation)
    for delta in [0.1, 0.5, 1.0, 1.5, 2.0, 2.5, 3.5, 5.0, 10.0]:
        configs.append({
            "id": cid, "phase": 1,
            "delta": delta, "target_pps": 0.0,
            "scale_up": 1.0, "scale_down": 1.0,
            "delta_min": 0.1, "delta_max": 100.0,
            "label": f"static delta={delta}",
        })
        cid += 1

    # Phase 2: adaptive policy sweep
    for delta_init, target_pps, scale_up, scale_down in product(
        [0.5, 1.0, 2.0],            # starting delta
        [50_000, 200_000, 500_000], # paths-per-step target
        [1.5, 2.5],                 # scale_up when sparse
        [0.7, 0.9],                 # scale_down when dense
    ):
        configs.append({
            "id": cid, "phase": 2,
            "delta": delta_init, "target_pps": float(target_pps),
            "scale_up": scale_up, "scale_down": scale_down,
            "delta_min": 0.1, "delta_max": 50.0,
            "label": (f"adaptive d0={delta_init} tpps={target_pps} "
                      f"up={scale_up} dn={scale_down}"),
        })
        cid += 1

    return configs


# ── config persistence ─────────────────────────────────────────────────────────
def load_or_init_config():
    if CONFIG_FILE.exists():
        with open(CONFIG_FILE) as f:
            return json.load(f)

    # First run: initialise with baseline + queue
    cfg = {
        "baseline": {
            "delta": 2.5, "target_pps": 0.0, "threads": NUM_THREADS,
            "note": "static delta=2.5, recorded before autoloop started",
        },
        "best": {"id": 0, "label": "baseline (delta=2.5)", "avg_ms": None},
        "queue": generate_queue(),
        "completed": [],
    }
    save_config(cfg)
    return cfg


def save_config(cfg):
    with open(CONFIG_FILE, "w") as f:
        json.dump(cfg, f, indent=2)


# ── running big-table ──────────────────────────────────────────────────────────
def run_benchmark(config, benchmark):
    """
    Run big-table for one benchmark with the given config.
    Returns dict with keys: avg_err, max_err, time_ms, num_steps, avg_pps,
                             delta_min, delta_max, ok (bool).
    """
    env = os.environ.copy()
    env["CPATHGEN_DELTA"]      = str(config["delta"])
    env["CPATHGEN_TARGET_PPS"] = str(config["target_pps"])
    env["CPATHGEN_SCALE_UP"]   = str(config["scale_up"])
    env["CPATHGEN_SCALE_DOWN"] = str(config["scale_down"])
    env["CPATHGEN_DELTA_MIN"]  = str(config["delta_min"])
    env["CPATHGEN_DELTA_MAX"]  = str(config["delta_max"])

    edges  = REPO / "benchmarks" / f"{benchmark}.edges"
    golden = REPO / "golden"     / f"{benchmark}.golden"
    csv_out = REPO / "big-table.csv"
    csv_out.unlink(missing_ok=True)

    proc = subprocess.run(
        [str(BIG_TABLE), str(K), str(edges), str(golden), str(NUM_THREADS)],
        env=env, cwd=REPO, capture_output=True, text=True,
    )

    stdout = proc.stdout
    result = {"ok": proc.returncode == 0}

    # Parse per-step stats from stdout
    def grab(key):
        m = re.search(rf"^{key}=(.+)$", stdout, re.MULTILINE)
        return m.group(1).strip() if m else None

    result["num_steps"] = int(grab("cpathgen_num_steps") or 0)
    result["avg_pps"]   = float(grab("cpathgen_avg_pps")  or 0.0)
    result["delta_min"] = float(grab("cpathgen_delta_min") or config["delta"])
    result["delta_max"] = float(grab("cpathgen_delta_max") or config["delta"])

    # Parse CSV for error + timing
    if csv_out.exists():
        with open(csv_out) as f:
            rows = list(csv.DictReader(f))
        if rows:
            row = rows[-1]
            # cpathgen_avg_pfxt_time field looks like "71.2(9.7$\times$)"
            t_field = row.get("cpathgen_avg_pfxt_time", "0") or "0"
            t_match = re.match(r"([\d.]+)", t_field)
            result["time_ms"]  = float(t_match.group(1)) if t_match else 0.0
            result["avg_err"]  = float(row.get("cpathgen_avg_err", "999") or "999")
            result["max_err"]  = float(row.get("cpathgen_max_err", "999") or "999")
        else:
            result.update({"time_ms": 0.0, "avg_err": 999.0, "max_err": 999.0})
    else:
        result.update({"time_ms": 0.0, "avg_err": 999.0, "max_err": 999.0})

    return result


# ── oracle + scoring ──────────────────────────────────────────────────────────
def compute_oracle(completed):
    """
    From Phase 1 completed entries, return the best (lowest) correct time_ms
    per benchmark.  Returns dict: {benchmark: {"time_ms": ..., "delta": ...}}
    """
    oracle = {}
    for entry in completed:
        if entry["config"]["phase"] != 1 or not entry["all_correct"]:
            continue
        for bm, r in entry["results"].items():
            if r["avg_err"] != 0.0 or r["max_err"] != 0.0:
                continue
            if bm not in oracle or r["time_ms"] < oracle[bm]["time_ms"]:
                oracle[bm] = {"time_ms": r["time_ms"],
                               "delta": entry["config"]["delta"]}
    return oracle


def score_vs_oracle(bm_results, oracle):
    """
    For each benchmark, compute ratio = time / oracle_time.
    Returns list of (bm, ratio, win) sorted by benchmark name.
    win = ratio <= WIN_THRESHOLD
    """
    scores = []
    for bm, r in bm_results.items():
        if bm not in oracle or r["avg_err"] != 0.0:
            scores.append((bm, None, False))
            continue
        ratio = r["time_ms"] / oracle[bm]["time_ms"]
        scores.append((bm, ratio, ratio <= WIN_THRESHOLD))
    return sorted(scores)


# ── progress log ───────────────────────────────────────────────────────────────
def append_round(cfg, config, bm_results, round_num):
    """Append a round entry to progress_log.md."""
    all_zero_err = all(r["avg_err"] == 0.0 and r["max_err"] == 0.0
                       for r in bm_results.values())
    avg_ms = (sum(r["time_ms"] for r in bm_results.values())
              / len(bm_results))

    lines = [
        f"\n---\n",
        f"## Round {round_num} — {config['label']}\n\n",
        f"**Phase {config['phase']}** | Config ID {config['id']}\n\n",
        f"### Parameters\n\n",
        f"| param | value |\n|---|---|\n",
        f"| delta_init | {config['delta']} |\n",
        f"| target_pps | {config['target_pps']} |\n",
        f"| scale_up | {config['scale_up']} |\n",
        f"| scale_down | {config['scale_down']} |\n",
        f"| delta_min | {config['delta_min']} |\n",
        f"| delta_max | {config['delta_max']} |\n\n",
        f"### Results (K=1M, 16 threads)\n\n",
        f"| benchmark | time_ms | avg_err | max_err | steps | avg_pps | d_min | d_max |\n",
        f"|---|---|---|---|---|---|---|---|\n",
    ]
    for bm, r in bm_results.items():
        lines.append(
            f"| {bm} | {r['time_ms']:.1f} | {r['avg_err']:.4f} | "
            f"{r['max_err']:.4f} | {r['num_steps']} | {r['avg_pps']:.0f} | "
            f"{r['delta_min']:.3f} | {r['delta_max']:.3f} |\n"
        )
    lines.append(f"\n**Avg time:** {avg_ms:.1f} ms\n")

    # Phase 2: score vs oracle
    if config["phase"] == 2:
        oracle = compute_oracle(cfg["completed"])
        if oracle:
            scores = score_vs_oracle(bm_results, oracle)
            wins = sum(1 for _, _, w in scores if w)
            lines.append(f"\n### Oracle comparison (win = within {int((WIN_THRESHOLD-1)*100)}% of best static delta)\n\n")
            lines.append(f"| benchmark | oracle_ms (best δ) | this_ms | ratio | win? |\n")
            lines.append(f"|---|---|---|---|---|\n")
            for bm, ratio, win in scores:
                o = oracle.get(bm, {})
                r = bm_results[bm]
                ratio_str = f"{ratio:.3f}" if ratio is not None else "—"
                win_str = "✅" if win else "❌"
                lines.append(
                    f"| {bm} | {o.get('time_ms','?'):.1f} (δ={o.get('delta','?')}) "
                    f"| {r['time_ms']:.1f} | {ratio_str} | {win_str} |\n"
                )
            lines.append(f"\n**Win rate: {wins}/{len(scores)} benchmarks**\n")

    with open(PROGRESS_LOG, "a") as f:
        f.writelines(lines)


def append_phase1_summary(cfg):
    """Write oracle table after Phase 1 is complete."""
    oracle = compute_oracle(cfg["completed"])
    lines = [
        "\n---\n\n## Phase 1 Complete — Oracle Established\n\n",
        "Best static delta per benchmark:\n\n",
        "| benchmark | oracle delta | oracle time_ms |\n|---|---|---|\n",
    ]
    for bm, o in sorted(oracle.items()):
        lines.append(f"| {bm} | {o['delta']} | {o['time_ms']:.1f} |\n")
    lines.append(
        f"\nPhase 2 (adaptive rules) will be scored against these oracle times.\n"
        f"**Win threshold: within {int((WIN_THRESHOLD-1)*100)}% of oracle.**\n"
    )
    with open(PROGRESS_LOG, "a") as f:
        f.writelines(lines)


def append_final_summary(cfg):
    """Write ranked summary of all Phase 2 rules after search is complete."""
    oracle = compute_oracle(cfg["completed"])
    if not oracle:
        return

    # Score every Phase 2 completed entry
    ranked = []
    for entry in cfg["completed"]:
        if entry["config"]["phase"] != 2 or not entry["all_correct"]:
            continue
        scores = score_vs_oracle(entry["results"], oracle)
        wins = sum(1 for _, _, w in scores if w)
        avg_ratio = sum(r for _, r, _ in scores if r is not None) / max(
            1, sum(1 for _, r, _ in scores if r is not None))
        ranked.append((wins, -avg_ratio, entry["config"], entry["avg_ms"], scores))

    ranked.sort(reverse=True)  # most wins first, then best avg ratio

    lines = [
        "\n---\n\n## Final Summary — Adaptive Rule Ranking\n\n",
        f"Oracle baseline (best static delta per benchmark): "
        + ", ".join(f"{bm}={o['time_ms']:.1f}ms(δ={o['delta']})" for bm,o in sorted(oracle.items()))
        + "\n\n",
        f"Win threshold: within {int((WIN_THRESHOLD-1)*100)}% of oracle.\n\n",
        "### Top adaptive rules\n\n",
        "| rank | label | wins | avg_ratio | d0 | tpps | scale_up | scale_dn |\n",
        "|---|---|---|---|---|---|---|---|\n",
    ]
    for i, (wins, neg_ratio, config, avg_ms, _) in enumerate(ranked[:10], 1):
        lines.append(
            f"| {i} | {config['label']} | {wins}/{len(oracle)} "
            f"| {-neg_ratio:.3f} | {config['delta']} | {config['target_pps']:.0f} "
            f"| {config['scale_up']} | {config['scale_down']} |\n"
        )

    if ranked:
        best_wins, _, best_cfg, _, best_scores = ranked[0]
        lines += [
            f"\n### Recommended universal rule\n\n",
            f"```\n",
            f"delta_init  = {best_cfg['delta']}\n",
            f"target_pps  = {best_cfg['target_pps']:.0f}\n",
            f"scale_up    = {best_cfg['scale_up']}\n",
            f"scale_down  = {best_cfg['scale_down']}\n",
            f"delta_min   = {best_cfg['delta_min']}\n",
            f"delta_max   = {best_cfg['delta_max']}\n",
            f"```\n",
            f"\nWins on {best_wins}/{len(oracle)} benchmarks "
            f"({100*best_wins//len(oracle)}%).\n",
        ]

    with open(PROGRESS_LOG, "a") as f:
        f.writelines(lines)


# ── git helpers ────────────────────────────────────────────────────────────────
def git_commit_push(round_num, config_label):
    subprocess.run(
        ["git", "add",
         "progress_log.md",
         "scripts/autoloop_config.json"],
        cwd=REPO, check=False,
    )
    subprocess.run(
        ["git", "commit", "-m",
         f"autoloop: round {round_num} — {config_label}"],
        cwd=REPO, check=False,
    )
    subprocess.run(["git", "push"], cwd=REPO, check=False)


# ── main loop ──────────────────────────────────────────────────────────────────
def main():
    cfg = load_or_init_config()

    if not cfg["queue"]:
        print("Queue exhausted — systematic search complete.")
        print(f"Best config: {cfg['best']['label']} @ {cfg['best']['avg_ms']:.1f} ms")
        return

    total_remaining = len(cfg["queue"])
    print(f"Starting autoloop: {total_remaining} configs remaining.")

    while cfg["queue"]:
        config = cfg["queue"].pop(0)
        round_num = len(cfg["completed"]) + 1
        print(f"\n[Round {round_num}] {config['label']}")

        bm_results = {}
        for bm in BENCHMARKS:
            print(f"  running {bm}...", end=" ", flush=True)
            r = run_benchmark(config, bm)
            bm_results[bm] = r
            status = "ok" if r["ok"] and r["avg_err"] == 0.0 else "ERR"
            print(f"{r['time_ms']:.1f}ms  steps={r['num_steps']}  {status}")

        avg_ms = sum(r["time_ms"] for r in bm_results.values()) / len(bm_results)
        all_correct = all(r["avg_err"] == 0.0 and r["max_err"] == 0.0
                          for r in bm_results.values())

        # Update best (only for correct runs)
        if all_correct:
            if cfg["best"]["avg_ms"] is None or avg_ms < cfg["best"]["avg_ms"] * 0.99:
                cfg["best"] = {
                    "id": config["id"],
                    "label": config["label"],
                    "avg_ms": avg_ms,
                    "config": config,
                }

        # Record completed
        cfg["completed"].append({
            "config": config,
            "results": bm_results,
            "avg_ms": avg_ms,
            "all_correct": all_correct,
        })

        # Update log and persist
        append_round(cfg, config, bm_results, round_num)

        # After the last Phase 1 config, write the oracle summary
        next_phase = cfg["queue"][0]["phase"] if cfg["queue"] else None
        if config["phase"] == 1 and next_phase == 2:
            append_phase1_summary(cfg)

        save_config(cfg)

        # Commit after each round so progress is visible even if interrupted
        git_commit_push(round_num, config["label"])

    # Final ranking table
    append_final_summary(cfg)
    save_config(cfg)
    git_commit_push("final", "complete — see Final Summary")

    if cfg["best"]["avg_ms"] is not None:
        print(f"\nAutoloop complete. Best: {cfg['best']['label']} @ {cfg['best']['avg_ms']:.1f} ms")
    else:
        print("\nAutoloop complete.")


if __name__ == "__main__":
    main()
