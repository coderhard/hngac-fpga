#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

ITERATIONS="${1:-20000}"
OUTPUT_DIR="${2:-/tmp/hngac-fpga-local-compare-sweep}"
shift $(( $# > 0 ? 1 : 0 )) || true
shift $(( $# > 0 ? 1 : 0 )) || true

if [ "$#" -gt 0 ]; then
  LOOKUP_DELAYS=("$@")
else
  LOOKUP_DELAYS=(1000 10000 100000 200000)
fi

mkdir -p "${OUTPUT_DIR}"
SUMMARY_FILE="${OUTPUT_DIR}/sweep_summary_$(date +%Y%m%d_%H%M%S).log"

{
  echo "Iterations: ${ITERATIONS}"
  echo "Lookup delays (ns): ${LOOKUP_DELAYS[*]}"
  echo
} | tee "${SUMMARY_FILE}"

for delay in "${LOOKUP_DELAYS[@]}"; do
  echo "=== lookup_delay_ns=${delay} ===" | tee -a "${SUMMARY_FILE}"
  bash "${SCRIPT_DIR}/run_local_compare.sh" "${ITERATIONS}" "${delay}" "${OUTPUT_DIR}/delay_${delay}" | tee -a "${SUMMARY_FILE}"
  echo | tee -a "${SUMMARY_FILE}"
done

echo "Saved sweep summary: ${SUMMARY_FILE}"
