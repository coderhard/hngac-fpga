#!/usr/bin/env bash
# Run the canonical comparison benchmark and save output to data/benchmarks/.
# Uses the pre-built binary at ~/local/projects/hngac-fpga-build/ if present,
# otherwise falls back to building from source.
#
# Usage: ./scripts/run_benchmark.sh [iterations] [rbac_lookup_delay_ns]
# Defaults: 200000 iterations, 100000 ns delay (canonical paper run)
#
# Output is always saved to:
#   data/benchmarks/benchmark_YYYYMMDD_HHMMSS.log   (raw output)
#   data/benchmarks/benchmark_YYYYMMDD_HHMMSS.meta  (env/command metadata)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ITERATIONS="${1:-200000}"
DELAY_NS="${2:-100000}"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
OUT_DIR="${REPO_ROOT}/data/benchmarks"
LOG="${OUT_DIR}/benchmark_${TIMESTAMP}.log"
META="${OUT_DIR}/benchmark_${TIMESTAMP}.meta"

mkdir -p "$OUT_DIR"

# Find binary — prefer local pre-built, fall back to /tmp build
LOCAL_BIN="${HOME}/local/projects/hngac-fpga-build/hngac_compare_benchmark"
TMP_BIN="/tmp/hngac-fpga-build/hngac_compare_benchmark"

if [[ -x "$LOCAL_BIN" ]]; then
    BIN="$LOCAL_BIN"
elif [[ -x "$TMP_BIN" ]]; then
    BIN="$TMP_BIN"
else
    echo "No pre-built binary found. Building from source..."
    cmake -S "${REPO_ROOT}/fpga/hls" -B /tmp/hngac-fpga-build -DCMAKE_BUILD_TYPE=Release
    cmake --build /tmp/hngac-fpga-build
    BIN="$TMP_BIN"
fi

# Write metadata before run so it's saved even if the run is killed
cat > "$META" <<EOF
timestamp: ${TIMESTAMP}
date: $(date '+%Y-%m-%d %H:%M:%S %Z')
host: $(hostname)
kernel: $(uname -r)
binary: ${BIN}
binary_mtime: $(stat -c '%y' "$BIN" 2>/dev/null || echo unknown)
iterations: ${ITERATIONS}
rbac_lookup_delay_ns: ${DELAY_NS}
git_commit: $(git -C "$REPO_ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)
git_branch: $(git -C "$REPO_ROOT" rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)
EOF

echo "=== hngac benchmark ${TIMESTAMP} ===" | tee "$LOG"
echo "Binary: ${BIN}" | tee -a "$LOG"
echo "Iterations: ${ITERATIONS} | RBAC delay: ${DELAY_NS} ns" | tee -a "$LOG"
echo "---" | tee -a "$LOG"

"$BIN" "$ITERATIONS" "$DELAY_NS" 2>&1 | tee -a "$LOG"

echo ""
echo "Saved: ${LOG}"
echo "Meta:  ${META}"
