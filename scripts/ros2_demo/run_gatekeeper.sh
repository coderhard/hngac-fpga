#!/usr/bin/env bash
# Terminal 1 — 5D Gatekeeper
# Attack Class 2 demo: enforces subject + provenance policy.
# Logs [PASS_5D], [BLOCK_PROV], [BLOCK_SUBJ] with per-event authorization latency (ns).
# On exit the node prints: total / passed / blocked-subject / blocked-provenance + block rate.
#
# Usage:
#   ./run_gatekeeper.sh [--log] [--duration N]
#
#   --log          Tee output to data/attack2_gatekeeper_<timestamp>.log
#   --duration N   Stop after N seconds (default: run until Ctrl-C)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
WS="$REPO_ROOT/ros2_ws"
DATA_DIR="$REPO_ROOT/data"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"

LOG=false
DURATION=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --log)      LOG=true; shift ;;
        --duration) DURATION="$2"; shift 2 ;;
        *) echo "Unknown flag: $1" >&2; exit 1 ;;
    esac
done

set +u
source /opt/ros/jazzy/setup.bash
source "$WS/install/setup.bash"
set -u

stamp_lines() {
    awk '{ printf "[%s] %s\n", strftime("%H:%M:%S"), $0; fflush() }'
}

echo "=== Attack Class 2: 5D Gatekeeper ==="
echo "Policy: subject=1, object=2, attr=5, provenance=authenticated_ros2_node(1), state=wildcard"
echo "Blocks: subject=1 + provenance=remote_operator(4) [compromised node]"
echo "Passes: subject=1 + provenance=authenticated_ros2_node(1) [legit node]"
if [[ -n "$DURATION" ]]; then echo "Duration: ${DURATION}s"; fi
if [[ "$LOG" == true ]]; then
    mkdir -p "$DATA_DIR"
    LOGFILE="$DATA_DIR/attack2_gatekeeper_${TIMESTAMP}.log"
    echo "Log: $LOGFILE"
fi
echo "---"

run_node() {
    if [[ -n "$DURATION" ]]; then
        timeout "$DURATION" ros2 run ngac_auth gatekeeper_5d_node || true
    else
        ros2 run ngac_auth gatekeeper_5d_node
    fi
}

if [[ "$LOG" == true ]]; then
    run_node 2>&1 | stamp_lines | tee "$LOGFILE"
else
    run_node 2>&1 | stamp_lines
fi
