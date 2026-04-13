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

echo "lookup_delay_ns,rbac_hash_mean_ns,ngac_dag_mean_ns,baseline3d_mean_ns,state4d_mean_ns,rbac_lookup_mean_ns,rbac_sqlite_mean_ns,rbac_hash_allowed,ngac_dag_allowed,baseline3d_allowed,state4d_allowed,rbac_lookup_allowed,rbac_sqlite_allowed,overhead_pct,hashmap_gap_pct,dag_slowdown_x,rbac_slowdown_x,rbac_sqlite_slowdown_x" > "${CSV_FILE}"

for delay in "${LOOKUP_DELAYS[@]}"; do
  echo "=== lookup_delay_ns=${delay} ===" | tee -a "${SUMMARY_FILE}"
  RUN_OUTPUT="$(bash "${SCRIPT_DIR}/run_local_compare.sh" "${ITERATIONS}" "${delay}" "${OUTPUT_DIR}/delay_${delay}")"
  echo "${RUN_OUTPUT}" | tee -a "${SUMMARY_FILE}"

  rbac_hash_line="$(echo "${RUN_OUTPUT}" | grep '^RBAC hash map:')"
  dag_line="$(echo "${RUN_OUTPUT}" | grep '^NGAC-DAG traversal:')"
  baseline_line="$(echo "${RUN_OUTPUT}" | grep '^3D baseline:')"
  state4d_line="$(echo "${RUN_OUTPUT}" | grep '^4D state-aware:')"
  rbac_line="$(echo "${RUN_OUTPUT}" | grep '^RBAC + state lookup:')"
  sqlite_line="$(echo "${RUN_OUTPUT}" | grep '^RBAC + SQLite state lookup:')"
  overhead_line="$(echo "${RUN_OUTPUT}" | grep '^4D vs 3D mean overhead:')"
  hashmap_gap_line="$(echo "${RUN_OUTPUT}" | grep '^4D vs RBAC hash-map mean overhead:')"
  dag_slowdown_line="$(echo "${RUN_OUTPUT}" | grep '^NGAC-DAG vs 4D mean slowdown:')"
  slowdown_line="$(echo "${RUN_OUTPUT}" | grep '^RBAC+lookup vs 4D mean slowdown:')"
  sqlite_slowdown_line="$(echo "${RUN_OUTPUT}" | grep '^RBAC+SQLite vs 4D mean slowdown:')"

  rbac_hash_mean="$(echo "${rbac_hash_line}" | sed -E 's/.*mean=([^ ]+) ns.*/\1/')"
  dag_mean="$(echo "${dag_line}" | sed -E 's/.*mean=([^ ]+) ns.*/\1/')"
  baseline3d_mean="$(echo "${RUN_OUTPUT}" | awk -F'mean=| ns p99=' '/^3D baseline:/ {print $2}')"
  state4d_mean="$(echo "${RUN_OUTPUT}" | awk -F'mean=| ns p99=' '/^4D state-aware:/ {print $2}')"
  rbac_mean="$(echo "${rbac_line}" | sed -E 's/.*mean=([^ ]+) ns.*/\1/')"
  sqlite_mean=""
  if echo "${sqlite_line}" | grep -q "mean="; then
    sqlite_mean="$(echo "${sqlite_line}" | sed -E 's/.*mean=([^ ]+) ns.*/\1/')"
  fi
  rbac_hash_allowed="$(echo "${rbac_hash_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  dag_allowed="$(echo "${dag_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  baseline3d_allowed="$(echo "${baseline_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  state4d_allowed="$(echo "${state4d_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  rbac_allowed="$(echo "${rbac_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  sqlite_allowed=""
  if echo "${sqlite_line}" | grep -q "allowed="; then
    sqlite_allowed="$(echo "${sqlite_line}" | sed -E 's/.*allowed=([^ ]+).*/\1/')"
  fi
  overhead_pct="$(echo "${overhead_line}" | sed -E 's/.*: ([^%]+)%.*/\1/')"
  hashmap_gap_pct="$(echo "${hashmap_gap_line}" | sed -E 's/.*: ([^%]+)%.*/\1/')"
  dag_slowdown="$(echo "${dag_slowdown_line}" | sed -E 's/.*: ([^x]+)x.*/\1/')"
  rbac_slowdown="$(echo "${slowdown_line}" | sed -E 's/.*: ([^x]+)x.*/\1/')"
  sqlite_slowdown=""
  if [ -n "${sqlite_slowdown_line}" ]; then
    sqlite_slowdown="$(echo "${sqlite_slowdown_line}" | sed -E 's/.*: ([^x]+)x.*/\1/')"
  fi

  echo "${delay},${rbac_hash_mean},${dag_mean},${baseline3d_mean},${state4d_mean},${rbac_mean},${sqlite_mean},${rbac_hash_allowed},${dag_allowed},${baseline3d_allowed},${state4d_allowed},${rbac_allowed},${sqlite_allowed},${overhead_pct},${hashmap_gap_pct},${dag_slowdown},${rbac_slowdown},${sqlite_slowdown}" >> "${CSV_FILE}"
  echo | tee -a "${SUMMARY_FILE}"
done

echo "Saved sweep summary: ${SUMMARY_FILE}"
echo "Saved sweep CSV: ${CSV_FILE}"
