#!/usr/bin/env bash
# Terminal 2 — Legitimate ROS2 node
# Subject=1, provenance=authenticated_ros2_node(1).
# All commands should PASS 5D — used to confirm zero false positives.
#
# Usage:
#   ./run_legit.sh [--log] [--duration N] [--rate HZ]
#
#   --log          Tee output to data/attack2_legit_<timestamp>.log
#   --duration N   Stop after N seconds (default: run until Ctrl-C)
#   --rate HZ      Publish rate in Hz (default: 2.0)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
WS="$REPO_ROOT/ros2_ws"
DATA_DIR="$REPO_ROOT/data"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"

LOG=false
DURATION=""
RATE="2.0"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --log)      LOG=true; shift ;;
        --duration) DURATION="$2"; shift 2 ;;
        --rate)     RATE="$2"; shift 2 ;;
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

echo "=== Attack Class 2: Legitimate Node ==="
echo "Subject=1, provenance=authenticated_ros2_node(1), rate=${RATE} Hz"
echo "Expected: 100% PASS at gatekeeper (false-positive check)"
if [[ -n "$DURATION" ]]; then echo "Duration: ${DURATION}s"; fi
if [[ "$LOG" == true ]]; then
    mkdir -p "$DATA_DIR"
    LOGFILE="$DATA_DIR/attack2_legit_${TIMESTAMP}.log"
    echo "Log: $LOGFILE"
fi
echo "---"

run_node() {
    if [[ -n "$DURATION" ]]; then
        timeout "$DURATION" ros2 run ngac_auth legit_ros2_node \
            --ros-args -p rate_hz:="$RATE" || true
    else
        ros2 run ngac_auth legit_ros2_node \
            --ros-args -p rate_hz:="$RATE"
    fi
}

if [[ "$LOG" == true ]]; then
    run_node 2>&1 | stamp_lines | tee "$LOGFILE"
else
    run_node 2>&1 | stamp_lines
fi
