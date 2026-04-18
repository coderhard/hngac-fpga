#!/usr/bin/env bash
# Build the ngac_auth ROS2 package (run once before any demo script).
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
WS="$REPO_ROOT/ros2_ws"

set +u; source /opt/ros/jazzy/setup.bash; set -u
cd "$WS"
colcon build --packages-select ngac_auth --cmake-args -DCMAKE_BUILD_TYPE=Release
echo "Build done. Source: source $WS/install/setup.bash"
