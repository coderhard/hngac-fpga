#!/usr/bin/env bash
# Reproduce the software benchmark with perf stat cycle counts.
#
# Builds hngac_compare_benchmark from the kernel source, then runs each model
# at each rule count under perf stat to capture cpu_core/cycles. Output goes
# to perf_all_models_scaling.log in the same directory as this script.
#
# Requirements:
#   - g++ with C++17 support
#   - cmake
#   - perf (linux-tools), with perf_event_paranoid <= 1
#   - run under Linux (WSL2 works; perf counters are real hardware counters)
#
# Usage:
#   bash run_perf_benchmark.sh [iterations] [output_file]
#   Default: 200000 iterations, output to ./perf_all_models_scaling.log

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ITERATIONS="${1:-200000}"
OUTPUT="${2:-${SCRIPT_DIR}/perf_all_models_scaling.log}"
BUILD_DIR="/tmp/hngac-build"

# Locate the kernel source -- try relative paths from the package layout
HLS_ROOT=""
for candidate in \
    "${SCRIPT_DIR}/../../kernel/5d/hngac-fpga/fpga/hls" \
    "${SCRIPT_DIR}/../../kernel/5d/fpga/hls" \
    "${SCRIPT_DIR}/../../../kernel/5d/hngac-fpga/fpga/hls"; do
    if [ -f "${candidate}/bench/hngac_compare_benchmark.cpp" ]; then
        HLS_ROOT="$(cd "${candidate}" && pwd)"
        break
    fi
done

if [ -z "${HLS_ROOT}" ]; then
    echo "ERROR: cannot find hngac_compare_benchmark.cpp"
    echo "Expected at kernel/5d/hngac-fpga/fpga/hls/bench/"
    exit 1
fi

echo "source:     ${HLS_ROOT}"
echo "iterations: ${ITERATIONS}"
echo "output:     ${OUTPUT}"
echo "build dir:  ${BUILD_DIR}"
echo

# Build
mkdir -p "${BUILD_DIR}"
cmake -S "${HLS_ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1
cmake --build "${BUILD_DIR}" --target hngac_compare_benchmark -- -j"$(nproc)" > /dev/null 2>&1
BIN="${BUILD_DIR}/hngac_compare_benchmark"

if [ ! -x "${BIN}" ]; then
    echo "ERROR: build failed, ${BIN} not found"
    exit 1
fi
echo "built: ${BIN}"
echo

# Check perf availability
if ! command -v perf &> /dev/null; then
    echo "ERROR: perf not found. Install linux-tools-\$(uname -r) or linux-tools-generic."
    exit 1
fi

PARANOID=$(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null || echo "?")
if [ "${PARANOID}" -gt 1 ] 2>/dev/null; then
    echo "WARNING: perf_event_paranoid=${PARANOID} (need <= 1)"
    echo "  Fix: echo 1 | sudo tee /proc/sys/kernel/perf_event_paranoid"
fi

# Models and rule counts matching the paper's benchmark corpus
MODELS=("H-NGAC 3D" "H-NGAC 4D" "H-NGAC 5D" "RBAC hash map" "NGAC-DAG" "Flattened")
RULE_COUNTS=(4 10 50 100 200 500)

> "${OUTPUT}"

for model in "${MODELS[@]}"; do
    echo "====== ${model} ======" | tee -a "${OUTPUT}"
    for rules in "${RULE_COUNTS[@]}"; do
        echo "=== ${rules} rules ===" | tee -a "${OUTPUT}"

        # Run the benchmark -- it prints SUMMARY and CYCLES lines to stdout
        # perf stat output goes to stderr, which we merge
        perf stat -e cpu_core/cycles/,cpu_core/instructions/ \
            "${BIN}" "${ITERATIONS}" 0 "${rules}" "${model}" \
            2>&1 | tee -a "${OUTPUT}"

        echo | tee -a "${OUTPUT}"
    done
done

echo "Done. Output: ${OUTPUT}"
