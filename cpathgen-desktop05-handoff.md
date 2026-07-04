# C-PathGen Revision Handoff for desktop-05

This machine replaces `twhuang-desktop-04`, which is down. Use `desktop-05` for the remaining revision experiments because it has the same class of hardware as `desktop-04`. Do not use `twhuang-server-01` performance numbers for paper figures; that 80-thread server showed machine-specific scaling behavior.


## Revision Context

This handoff is for the TODAES journal revision of C-PathGen. The added experiments are driven mainly by Referee 3, who asked for stronger evidence beyond the authors' prior PathGen baseline and beyond the prefix-tree kernel-only evaluation. The key reviewer concerns are:

- Position C-PathGen relative to independent timing systems, especially Tatum and OpenSTA.
- Add practical end-to-end path-query timing, including preparation, generation, filtering/sorting, and recovered path traces where applicable.
- Strengthen exactness validation using ordered recovered path traces, not only path costs.
- Distinguish stress-test `k=1M` results from practical smaller `k` query sizes.
- Show CPU thread scaling and saturation points instead of only one fixed thread count.
- Report memory usage, using peak RSS.
- Add or update manuscript discussion/table comparing CPU C-PathGen, GPU path generation, and incremental timing work.

For the remaining experiments, focus on what can be run cleanly and reproducibly on `desktop-05`:

- C-PathGen/OpenTimer/PathGen can be measured now.
- OpenSTA and Tatum require different harnesses and should not block the core C-PathGen sweeps unless those harnesses are already ready. Treat them as discussion/limited-scope comparison work if needed.
- The GPU and incremental approaches are primarily a manuscript comparison table unless a working benchmark harness is already available.

Benchmark set for the circuit experiments:

```text
des_perf   smallest useful benchmark
vga_lcd    next small/medium benchmark
leon2      large benchmark
leon3mp    large benchmark
netcard    large benchmark
```

Do not use `tv80`; it is too small for these revision figures. Do not use `aes_core`; `des_perf` is the smallest benchmark we want.

Important implementation/history notes:

- Path trace recovery is exposed through `report_paths_mlq(..., recover_paths=true)` and uses recovered full traces.
- Exactness should compare complete ordered top-k trace sets where practical: missing paths, extra paths, duplicates, trace mismatches, cost mismatches, and ordering mismatches.
- Equal-cost paths may expose tie-order limitations; report exact ordered match when it succeeds, but distinguish true correctness failures from deterministic tie policy limitations.
- Zero-cost paths should be excluded in exactness checks where they are structural artifacts and not considered timing paths.
- Use `report_rebuild` for golden/reference exactness where possible; avoid relying on buggy incremental `report_incsfxt` as a correctness oracle.
- For C-PathGen paper-style settings, use `delta=2.5` and `num_vecs=20`.
- The current optimization commit changes `_spur_mlq` expansion to use per-thread local child buffers and bulk-flush them into MLQ vectors. It is still C-PathGen; it does not switch to PathGen's `_spur_multiq`.

Expected story from old-machine-class hardware:

- `k=1M` is a stress test.
- Thread curves should scale up to the hardware-thread/core limit and then plateau under oversubscription.
- Smaller `k` sweeps should show practical latency and identify where C-PathGen starts beating sequential/OpenTimer-style baselines.
- Use `desktop-05` numbers for figures; `twhuang-server-01` showed poor/unstable scaling and should only be mentioned as an internal debugging datapoint if at all.

## Goal

Run clean old-machine-style C-PathGen experiments for the journal revision:

- k=1M thread saturation sweeps on circuit benchmarks.
- k sweep at the saturation thread count.
- peak RSS measurements for C-PathGen, OpenTimer, and PathGen.
- exactness spot/full validation after the MLQ local-buffer optimization.

The current `main` branch includes the relevant code:

- `_spur_mlq` local child buffering to reduce concurrent bucket insertion contention.
- `examples/cpathgen/cpathgen-only-sweep.cpp`.
- `examples/cpathgen/run-cpathgen-thread-sweep-k1m.sh`.
- exactness helper binaries/sources.

## Clean Checkout

On `desktop-05`:

```bash
mkdir -p /home/cchang289/Research
cd /home/cchang289/Research

git clone https://github.com/Randy1005/Ink.git
cd Ink

git submodule update --init 3rd-party/OpenTimer 3rd-party/concurrentqueue 3rd-party/MPMCQueue
```

If HTTPS authentication is awkward, use the SSH remote if that machine has GitHub keys configured:

```bash
git clone git@github.com:Randy1005/Ink.git
```

## Copy Benchmarks

`desktop-04` is down. Copy the circuit `.edges` benchmarks from `twhuang-server-01`, where they are cached under the imported benchmark directory.

Run from `desktop-05`:

```bash
cd /home/cchang289/Research/Ink
mkdir -p benchmarks results logs

for b in des_perf vga_lcd leon2 leon3mp netcard; do
  scp "cchang289@twhuang-server-01.ece.wisc.edu:/home/cchang289/cpathgen_revision/benchmarks/imported/twhuang-desktop-04/ink/${b}.edges" \
    "benchmarks/${b}.edges"
done

ls -lh benchmarks/*.edges
```

Expected approximate sizes:

```text
des_perf.edges   46M
vga_lcd.edges    51M
leon3mp.edges   545M
netcard.edges   625M
leon2.edges     705M
```

If a multiline copy command is misparsed by Bash, copy files one at a time with the remote path quoted:

```bash
scp 'cchang289@twhuang-server-01.ece.wisc.edu:/home/cchang289/cpathgen_revision/benchmarks/imported/twhuang-desktop-04/ink/leon2.edges' benchmarks/leon2.edges
```

## Build

Avoid Conda compilers/TBB if possible. Use system compilers and system TBB.

```bash
cd /home/cchang289/Research/Ink
conda deactivate 2>/dev/null || true
unset LD_LIBRARY_PATH
hash -r

cmake -S . -B build-revision \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/usr/bin/gcc \
  -DCMAKE_CXX_COMPILER=/usr/bin/g++

cmake --build build-revision \
  --target cpathgen-only-sweep exactness-batch gen-golden-trace dump-paths kernel-sweep \
  --parallel
```

If CMake links against an incompatible `/usr/local/lib/libtbb.so`, reconfigure with:

```bash
rm -rf build-revision
cmake -S . -B build-revision \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/usr/bin/gcc \
  -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
  -DCMAKE_IGNORE_PREFIX_PATH=/usr/local \
  -DCMAKE_IGNORE_PATH=/usr/local/lib
```

## Experiment 1: k=1M Thread Saturation

Run C-PathGen-only kernel sweeps. Use 32 and 40 as oversubscription points if `desktop-05` has only 20 hardware threads; the expected story is scaling through the hardware limit and plateau afterward.

```bash
cd /home/cchang289/Research/Ink
mkdir -p results/thread_sweep logs

for b in des_perf vga_lcd leon2 leon3mp netcard; do
  THREADS=1,2,4,8,12,16,20,24,32,40 \
  examples/cpathgen/run-cpathgen-thread-sweep-k1m.sh \
    "benchmarks/${b}.edges" \
    "results/thread_sweep/${b}.cpathgen.k1m.thread_sweep.csv" \
    > "logs/${b}.thread_sweep.stdout.log" \
    2> "logs/${b}.thread_sweep.stderr.log"
done
```

The runner defaults are:

```text
K=1000000
DELTA=2.5
NUM_VECS=20
WARMUPS=1
TRIALS=10
THREADS=1,2,4,8,16,20,32,40,64,80
```

Override any default with environment variables, as shown above.

Summarize each CSV:

```bash
python3 - <<'PY'
import csv, glob, statistics as st
for path in sorted(glob.glob('results/thread_sweep/*.csv')):
    rows = list(csv.DictReader(open(path)))
    print('\n' + path)
    base = None
    for t in sorted({int(r['threads']) for r in rows}):
        xs = [float(r['kernel_ms']) for r in rows if int(r['threads']) == t]
        mean = st.mean(xs)
        sd = st.stdev(xs) if len(xs) > 1 else 0.0
        if t == 1:
            base = mean
        print(f'{t:>3}t n={len(xs):>2} mean={mean:9.3f} ms stdev={sd:8.3f} min={min(xs):9.3f} max={max(xs):9.3f} speedup={base/mean:5.2f}')
PY
```

## Experiment 2: k Sweep

Use the saturation thread count. Start with `THREADS=20` unless the thread sweep clearly shows a different best point. Run:

```bash
cd /home/cchang289/Research/Ink
mkdir -p results/k_sweep logs

for b in des_perf vga_lcd leon2 leon3mp netcard; do
  for k in 10 100 1000 10000 100000 1000000; do
    /usr/bin/time -v examples/cpathgen/cpathgen-only-sweep \
      "benchmarks/${b}.edges" \
      "results/k_sweep/${b}.cpathgen.k_sweep.csv" \
      "$k" \
      "20" \
      "1" \
      "10" \
      "2.5" \
      "20" \
      > "logs/${b}.k${k}.stdout.log" \
      2> "logs/${b}.k${k}.stderr.log"
  done
done
```

This rereads the graph per `k`. If runtime becomes too high, implement or reuse an in-process k-sweep runner that reads each graph once and loops over k values with reset between runs.

## Experiment 3: Peak RSS

Use `/usr/bin/time -v` and record `Maximum resident set size` as peak RSS.

C-PathGen peak RSS at k=1M, saturation threads:

```bash
mkdir -p results/memory logs
for b in des_perf vga_lcd leon2 leon3mp netcard; do
  /usr/bin/time -v examples/cpathgen/cpathgen-only-sweep \
    "benchmarks/${b}.edges" \
    "results/memory/${b}.cpathgen.memory.csv" \
    "1000000" "20" "1" "3" "2.5" "20" \
    > "logs/${b}.cpathgen.memory.stdout.log" \
    2> "logs/${b}.cpathgen.memory.stderr.log"
done
```

For OpenTimer and PathGen, use the existing resource scripts in `experiments/scripts/` if available and working. The metric to extract is still `/usr/bin/time -v` peak RSS.

## Experiment 4: Exactness Validation

Set the revision root expected by the Python scripts:

```bash
export CPG_REVISION_ROOT=/home/cchang289/cpathgen_revision
```

If this machine only has `/home/cchang289/Research/Ink` and no `/home/cchang289/cpathgen_revision`, either create the root or set `CPG_REVISION_ROOT` to a directory that contains the expected results layout. If in doubt, inspect `experiments/scripts/common.py`.

Run spot checks first:

```bash
cd /home/cchang289/Research/Ink
mkdir -p results/exactness_spot

for b in des_perf vga_lcd leon2; do
  examples/cpathgen/exactness-batch \
    "benchmarks/${b}.edges" \
    "results/exactness_spot/${b}" \
    "$b" \
    "1000" \
    "100,1000" \
    "20,40"
done
```

Then validate each generated candidate with:

```bash
python3 experiments/scripts/validate_exactness.py \
  --golden results/exactness_spot/des_perf/golden.k1000.nozero.trace.jsonl \
  --candidate results/exactness_spot/des_perf/cpathgen.k1000.nozero.t20.run0.jsonl \
  --output results/exactness_spot/des_perf.validate.k1000.t20.csv \
  --benchmark des_perf --algorithm cpathgen --k 1000 --threads 20 --run-id 0
```

Scale this into a loop after the first validation succeeds. Required success criteria:

- `status=success`
- `exact_set_match=True`
- `exact_ordered_match=True`
- zero missing/extra/cost/order mismatches

Duplicate paths may be reported because of equal traces/cost ties; do not treat duplicates alone as a failure if set/order match is true.

## What To Tell Codex on desktop-05

Paste this prompt to Codex in `/home/cchang289/Research/Ink`:

```text
Read cpathgen-desktop05-handoff.md and continue the C-PathGen journal revision experiments on this machine. Do not use twhuang-server-01 runtime numbers for figures. First confirm the repo is on latest main and build cpathgen-only-sweep. Then copy missing benchmarks from twhuang-server-01 if needed. Run k=1M C-PathGen thread sweeps for des_perf, vga_lcd, leon2, leon3mp, netcard with THREADS=1,2,4,8,12,16,20,24,32,40, WARMUPS=1, TRIALS=10, DELTA=2.5, NUM_VECS=20. Summarize mean/stdev/min/max kernel_ms and saturation points. Then run k sweep, peak RSS, and exactness validation as described in the handoff doc. Keep generated result CSVs under results/ and logs under logs/; do not commit generated binaries or large result files unless explicitly asked.
```

## Notes From Prior Runs

On the old `desktop-04`-class machine, after `_spur_mlq` local buffering, `des_perf` k=1M had clean scaling:

```text
1t  ~207 ms
2t  ~117 ms
4t   ~75 ms
8t   ~56 ms
16t  ~47 ms
20t  ~45 ms
32t+ ~45 ms plateau/oversubscription
```

That is the kind of curve expected from `desktop-05` if hardware and toolchain are comparable.
