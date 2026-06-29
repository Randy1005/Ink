#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

BENCHMARK=${1:-${DES_PERF_EDGES:-des_perf.edges}}
OUTPUT=${2:-${CPATHGEN_ONLY_OUT:-cpathgen-only-des_perf-k1m.csv}}

K=${K:-1000000}
THREADS=${THREADS:-1,2,4,8,16,20,32,40,64,80}
WARMUPS=${WARMUPS:-1}
TRIALS=${TRIALS:-10}
DELTA=${DELTA:-2.5}
NUM_VECS=${NUM_VECS:-20}

if [[ ! -x "${SCRIPT_DIR}/cpathgen-only-sweep" ]]; then
  echo "missing ${SCRIPT_DIR}/cpathgen-only-sweep; build examples first" >&2
  exit 1
fi

/usr/bin/time -v "${SCRIPT_DIR}/cpathgen-only-sweep" \
  "${BENCHMARK}" \
  "${OUTPUT}" \
  "${K}" \
  "${THREADS}" \
  "${WARMUPS}" \
  "${TRIALS}" \
  "${DELTA}" \
  "${NUM_VECS}"
