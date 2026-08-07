#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

ITERATIONS="${1:-20000}"
OUTPUT_DIR="${2:-${REPO_ROOT}/data/benchmarks}"
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

echo "lookup_delay_ns,rbac_hash_mean_ns,ngac_dag_mean_ns,hngac3d_mean_ns,hngac4d_mean_ns,hngac5d_mean_ns,flattened5d_mean_ns,rbac_lookup_mean_ns,rbac_sqlite_mean_ns,rbac_hash_allowed,ngac_dag_allowed,hngac3d_allowed,hngac4d_allowed,hngac5d_allowed,flattened5d_allowed,rbac_lookup_allowed,rbac_sqlite_allowed,overhead_4d_vs_3d_pct,overhead_5d_vs_4d_pct,hngac5d_vs_flattened5d_mean_overhead_pct,hngac5d_vs_flattened5d_p99_ratio_x,flattened5d_memory_vs_hngac5d_x,flattened5d_reload_vs_hngac5d_x,ngacdag_vs_hngac5d_mean_slowdown_x,rbaclookup_vs_hngac5d_mean_slowdown_x,rbacsqlite_vs_hngac5d_mean_slowdown_x" > "${CSV_FILE}"

summary_field() {
  local output="$1"
  local label="$2"
  local field="$3"
  awk -F'|' -v label="${label}" -v field="${field}" \
    '$1 == "SUMMARY" && $2 == label {print $field; found=1} END {if (!found) print ""}' \
    <<< "${output}"
}

compare_field() {
  local output="$1"
  local key="$2"
  awk -F'|' -v key="${key}" \
    '$1 == "COMPARE" && $2 == key {print $3; found=1} END {if (!found) print ""}' \
    <<< "${output}"
}

for delay in "${LOOKUP_DELAYS[@]}"; do
  echo "=== lookup_delay_ns=${delay} ===" | tee -a "${SUMMARY_FILE}"
  RUN_OUTPUT="$(bash "${SCRIPT_DIR}/run_local_compare.sh" "${ITERATIONS}" "${delay}" "${OUTPUT_DIR}/delay_${delay}")"
  echo "${RUN_OUTPUT}" | tee -a "${SUMMARY_FILE}"

  rbac_hash_mean="$(summary_field "${RUN_OUTPUT}" "RBAC hash map" 3)"
  dag_mean="$(summary_field "${RUN_OUTPUT}" "NGAC-DAG traversal" 3)"
  hngac3d_mean="$(summary_field "${RUN_OUTPUT}" "H-NGAC 3D" 3)"
  hngac4d_mean="$(summary_field "${RUN_OUTPUT}" "H-NGAC 4D" 3)"
  hngac5d_mean="$(summary_field "${RUN_OUTPUT}" "H-NGAC 5D" 3)"
  flattened5d_mean="$(summary_field "${RUN_OUTPUT}" "Flattened 5D direct lookup" 3)"
  rbac_mean="$(summary_field "${RUN_OUTPUT}" "RBAC + state lookup" 3)"
  sqlite_mean="$(summary_field "${RUN_OUTPUT}" "RBAC + SQLite state lookup" 3)"

  rbac_hash_allowed="$(summary_field "${RUN_OUTPUT}" "RBAC hash map" 8)"
  dag_allowed="$(summary_field "${RUN_OUTPUT}" "NGAC-DAG traversal" 8)"
  hngac3d_allowed="$(summary_field "${RUN_OUTPUT}" "H-NGAC 3D" 8)"
  hngac4d_allowed="$(summary_field "${RUN_OUTPUT}" "H-NGAC 4D" 8)"
  hngac5d_allowed="$(summary_field "${RUN_OUTPUT}" "H-NGAC 5D" 8)"
  flattened5d_allowed="$(summary_field "${RUN_OUTPUT}" "Flattened 5D direct lookup" 8)"
  rbac_allowed="$(summary_field "${RUN_OUTPUT}" "RBAC + state lookup" 8)"
  sqlite_allowed="$(summary_field "${RUN_OUTPUT}" "RBAC + SQLite state lookup" 8)"

  overhead_4d_vs_3d="$(compare_field "${RUN_OUTPUT}" "4D_vs_3D_mean_overhead_pct")"
  overhead_5d_vs_4d="$(compare_field "${RUN_OUTPUT}" "5D_vs_4D_mean_overhead_pct")"
  hngac5d_vs_flattened="$(compare_field "${RUN_OUTPUT}" "HNGAC5D_vs_flattened5D_mean_overhead_pct")"
  hngac5d_vs_flattened_p99="$(compare_field "${RUN_OUTPUT}" "HNGAC5D_vs_flattened5D_p99_ratio_x")"
  flattened_memory="$(compare_field "${RUN_OUTPUT}" "flattened5D_memory_vs_HNGAC5D_x")"
  flattened_reload="$(compare_field "${RUN_OUTPUT}" "flattened5D_reload_vs_HNGAC5D_x")"
  dag_slowdown="$(compare_field "${RUN_OUTPUT}" "NGACDAG_vs_HNGAC5D_mean_slowdown_x")"
  rbac_slowdown="$(compare_field "${RUN_OUTPUT}" "RBAClookup_vs_HNGAC5D_mean_slowdown_x")"
  sqlite_slowdown="$(compare_field "${RUN_OUTPUT}" "RBACSQLite_vs_HNGAC5D_mean_slowdown_x")"

  echo "${delay},${rbac_hash_mean},${dag_mean},${hngac3d_mean},${hngac4d_mean},${hngac5d_mean},${flattened5d_mean},${rbac_mean},${sqlite_mean},${rbac_hash_allowed},${dag_allowed},${hngac3d_allowed},${hngac4d_allowed},${hngac5d_allowed},${flattened5d_allowed},${rbac_allowed},${sqlite_allowed},${overhead_4d_vs_3d},${overhead_5d_vs_4d},${hngac5d_vs_flattened},${hngac5d_vs_flattened_p99},${flattened_memory},${flattened_reload},${dag_slowdown},${rbac_slowdown},${sqlite_slowdown}" >> "${CSV_FILE}"
  echo | tee -a "${SUMMARY_FILE}"
done

echo "Saved sweep summary: ${SUMMARY_FILE}"
echo "Saved sweep CSV: ${CSV_FILE}"
