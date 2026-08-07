#!/usr/bin/env bash
# Terminal 3 — Compromised authenticated node
# Subject=1 (valid ID, same as legit), provenance=remote_operator(4).
# Would PASS under 3D/4D. BLOCKED by 5D. This is command provenance abuse.
#
# Usage:
#   ./run_compromised.sh [--log] [--duration N] [--rate HZ]
#
#   --log          Tee output to data/attack2_compromised_<timestamp>.log
#                  (attack2_ is the retired numbering for the command-provenance-abuse
#                   class; kept because committed evidence logs use it)
#   --duration N   Stop after N seconds (default: run until Ctrl-C)
#   --rate HZ      Inject rate in Hz (default: 10.0)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
WS="$REPO_ROOT/ros2_ws"
DATA_DIR="$REPO_ROOT/data"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"

LOG=false
DURATION=""
RATE="10.0"

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

echo "=== Command provenance abuse: Compromised Authenticated Node ==="
echo "Subject=1 (authorized ID), provenance=remote_operator(4), rate=${RATE} Hz"
echo "Expected: 100% BLOCK_PROV at gatekeeper under 5D enforcement"
echo "Note: these would PASS under 3D or 4D — provenance check is 5D-only"
if [[ -n "$DURATION" ]]; then echo "Duration: ${DURATION}s"; fi
if [[ "$LOG" == true ]]; then
    mkdir -p "$DATA_DIR"
    LOGFILE="$DATA_DIR/attack2_compromised_${TIMESTAMP}.log"
    echo "Log: $LOGFILE"
fi
echo "---"

run_node() {
    if [[ -n "$DURATION" ]]; then
        timeout "$DURATION" ros2 run ngac_auth compromised_ros2_node \
            --ros-args -p rate_hz:="$RATE" || true
    else
        ros2 run ngac_auth compromised_ros2_node \
            --ros-args -p rate_hz:="$RATE"
    fi
}

if [[ "$LOG" == true ]]; then
    run_node 2>&1 | stamp_lines | tee "$LOGFILE"
else
    run_node 2>&1 | stamp_lines
fi
