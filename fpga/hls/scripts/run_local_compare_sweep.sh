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
CSV_FILE="${OUTPUT_DIR}/sweep_summary.csv"

{
  echo "Iterations: ${ITERATIONS}"
  echo "Lookup delays (ns): ${LOOKUP_DELAYS[*]}"
  echo
} | tee "${SUMMARY_FILE}"

echo "lookup_delay_ns,baseline3d_mean_ns,state4d_mean_ns,rbac_lookup_mean_ns,baseline3d_allowed,state4d_allowed,rbac_lookup_allowed,overhead_pct,rbac_slowdown_x" > "${CSV_FILE}"

for delay in "${LOOKUP_DELAYS[@]}"; do
  echo "=== lookup_delay_ns=${delay} ===" | tee -a "${SUMMARY_FILE}"
  RUN_OUTPUT="$(bash "${SCRIPT_DIR}/run_local_compare.sh" "${ITERATIONS}" "${delay}" "${OUTPUT_DIR}/delay_${delay}")"
  echo "${RUN_OUTPUT}" | tee -a "${SUMMARY_FILE}"

  baseline_line="$(echo "${RUN_OUTPUT}" | grep '^3D baseline:')"
  state4d_line="$(echo "${RUN_OUTPUT}" | grep '^4D state-aware:')"
  rbac_line="$(echo "${RUN_OUTPUT}" | grep '^RBAC + state lookup:')"
  overhead_line="$(echo "${RUN_OUTPUT}" | grep '^4D vs 3D mean overhead:')"
  slowdown_line="$(echo "${RUN_OUTPUT}" | grep '^RBAC+lookup vs 4D mean slowdown:')"

  baseline3d_mean="$(echo "${RUN_OUTPUT}" | awk -F'mean=| ns p99=' '/^3D baseline:/ {print $2}')"
  state4d_mean="$(echo "${RUN_OUTPUT}" | awk -F'mean=| ns p99=' '/^4D state-aware:/ {print $2}')"
  rbac_mean="$(echo "${rbac_line}" | sed -E 's/.*mean=([^ ]+) ns.*/\1/')"
  baseline3d_allowed="$(echo "${baseline_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  state4d_allowed="$(echo "${state4d_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  rbac_allowed="$(echo "${rbac_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  overhead_pct="$(echo "${overhead_line}" | sed -E 's/.*: ([^%]+)%.*/\1/')"
  rbac_slowdown="$(echo "${slowdown_line}" | sed -E 's/.*: ([^x]+)x.*/\1/')"

  echo "${delay},${baseline3d_mean},${state4d_mean},${rbac_mean},${baseline3d_allowed},${state4d_allowed},${rbac_allowed},${overhead_pct},${rbac_slowdown}" >> "${CSV_FILE}"
  echo | tee -a "${SUMMARY_FILE}"
done

echo "Saved sweep summary: ${SUMMARY_FILE}"
echo "Saved sweep CSV: ${CSV_FILE}"
