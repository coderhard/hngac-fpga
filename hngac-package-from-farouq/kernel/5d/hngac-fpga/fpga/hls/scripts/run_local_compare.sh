#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HLS_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

ITERATIONS="${1:-20000}"
LOOKUP_DELAY_NS="${2:-100000}"
OUTPUT_DIR="${3:-${REPO_ROOT}/data/benchmarks}"

BUILD_DIR="${OUTPUT_DIR}/build"
LOG_FILE="${OUTPUT_DIR}/compare_$(date +%Y%m%d_%H%M%S).log"

mkdir -p "${BUILD_DIR}"

cmake -S "${HLS_ROOT}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}"

"${BUILD_DIR}/hngac_compare_benchmark" "${ITERATIONS}" "${LOOKUP_DELAY_NS}" | tee "${LOG_FILE}"

echo "Saved log: ${LOG_FILE}"
