#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Drop stale overlay paths from old device/location to avoid colcon prefix warnings.
unset AMENT_PREFIX_PATH
unset CMAKE_PREFIX_PATH
unset COLCON_PREFIX_PATH

source /opt/ros/humble/setup.bash

# Build native library required by tm_node_pkg.
# make -C "$PROJECT_ROOT/TM_V2" clean
# make -C "$PROJECT_ROOT/TM_V2"

rm -rf build/ log/ install/

colcon build --packages-select my_msgs_pkg
source install/setup.bash
colcon build
