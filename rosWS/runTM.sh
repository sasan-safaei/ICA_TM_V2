#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$SCRIPT_DIR"


#./setGPIO.sh
setup_gpio() {
	local gpio="$1"
	local direction="$2"
	local value="$3"
	local gpio_path="/sys/class/gpio/gpio${gpio}"

	# Export only when missing; ignore races where another process exports it first.
	if [[ ! -d "$gpio_path" ]]; then
		echo "$gpio" > /sys/class/gpio/export 2>/dev/null || true
		sleep 0.1
	fi

	echo "$direction" > "$gpio_path/direction"
	echo "$value" > "$gpio_path/value"
}

# SWD-uC-RESET (High)
setup_gpio 26 out 0
# UART-Tx-Active (High)
setup_gpio 19 out 1


export TM_WORKSPACE="$PROJECT_ROOT/TM_WorkSpace/"

export RCUTILS_CONSOLE_OUTPUT_FORMAT="[{severity}] [{name}]: {message}"
# Avoid loading ~/.local Python packages that can conflict with ROS/apt packages.
export PYTHONNOUSERSITE=1
source /opt/ros/humble/setup.bash
source install/setup.bash

#***************************************
export ROS_DOMAIN_ID=1
#***************************************
#export ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-1}

# Set StoreFolder in config based on ROS_DOMAIN_ID
STORE_FOLDER="./TestMachine$(printf '%03d' $ROS_DOMAIN_ID)"
sed -i "s|StoreFolder = .*|StoreFolder = $STORE_FOLDER|" "$TM_WORKSPACE/config.cfg"
#QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node tm_node_pkg tm_node

# Start tm_node in the background
ros2 run tm_node_pkg tm_node &
# Start gui_node with the QT framebuffer environment
QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node
