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


# ── progress log ───────────────────────────────────────────────────────────────
def append_round(cfg, config, bm_results, round_num):
    """Append a round entry to progress_log.md."""
    all_zero_err = all(r["avg_err"] == 0.0 and r["max_err"] == 0.0
                       for r in bm_results.values())
    avg_ms = (sum(r["time_ms"] for r in bm_results.values())
              / len(bm_results))
    best_ms = cfg["best"]["avg_ms"]
    if best_ms is None:
        verdict = "⬜ first result"
    elif not all_zero_err:
        verdict = "❌ error > 0"
    elif avg_ms < best_ms * 0.99:
        verdict = f"✅ improvement  ({best_ms:.1f} → {avg_ms:.1f} ms avg)"
    elif avg_ms > best_ms * 1.01:
        verdict = f"🔻 regression   ({best_ms:.1f} → {avg_ms:.1f} ms avg)"
    else:
        verdict = f"➖ plateau      ({avg_ms:.1f} ms avg)"

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
    lines += [
        f"\n**Avg time across benchmarks:** {avg_ms:.1f} ms\n\n",
        f"**Verdict:** {verdict}\n\n",
        f"**Best so far:** {cfg['best']['label']} @ {cfg['best']['avg_ms'] or 'TBD'} ms\n",
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
        save_config(cfg)

        # Commit after each round so progress is visible even if the run is interrupted
        git_commit_push(round_num, config["label"])

    print(f"\nAutoloop complete. Best: {cfg['best']['label']} @ {cfg['best']['avg_ms']:.1f} ms")


if __name__ == "__main__":
    main()
