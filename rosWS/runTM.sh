#!/bin/bash
set -e

# Forward script logs to journald when started by systemd.
if command -v systemd-cat >/dev/null 2>&1; then
	exec > >(systemd-cat -t runTM -p info) 2> >(systemd-cat -t runTM -p err)
fi

echo "Start runTM.sh"

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

# Default to kiosk framebuffer mode on server installs, but allow override from service env.
: "${QT_QPA_PLATFORM:=linuxfb}"
export QT_QPA_PLATFORM

# Ensure Qt has a runtime directory when running without a desktop session.
if [[ -z "${XDG_RUNTIME_DIR:-}" ]]; then
	export XDG_RUNTIME_DIR="/tmp/runtime-${USER}"
	mkdir -p "$XDG_RUNTIME_DIR"
	chmod 700 "$XDG_RUNTIME_DIR" 2>/dev/null || true
fi

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
#ros2 run tm_node_pkg tm_node &
# Start gui_node with the QT framebuffer environment
#QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node



#ros2 run tm_node_pkg tm_node >> /tmp/tm_node.log 2>&1 &
if ! pgrep -f "tm_node_pkg tm_node" > /dev/null; then
    ros2 run tm_node_pkg tm_node &
else
    echo "tm_node already running"
fi

if ! pgrep -f "qt_gui_node_pkg gui_node" > /dev/null; then
	while true; do
		echo "Start gui_node" #>> /tmp/gui_node.log
		ros2 run qt_gui_node_pkg gui_node #\
			#>> /tmp/gui_node.log 2>&1

		echo "$(date): gui_node stopped. Restarting..." #>> /tmp/gui_node.log
		sleep 2
    done
else
    echo "gui_node already running"
fi

