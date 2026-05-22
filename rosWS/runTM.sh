#!/bin/bash
cd ~/git_sasan/ICA_TM_V2/rosWS


#./setGPIO.sh
# SWD-uC-RESET (High)
echo 26 > /sys/class/gpio/export
# wait for sysfs to create the gpio folder
sleep 0.1
echo out > /sys/class/gpio/gpio26/direction
echo 0 > /sys/class/gpio/gpio26/value
# UART-Tx-Active (High)
echo 19 > /sys/class/gpio/export
# wait for sysfs to create the gpio folder
sleep 0.1
echo out > /sys/class/gpio/gpio19/direction
echo 1 > /sys/class/gpio/gpio19/value


export TM_WORKSPACE=~/git_sasan/ICA_TM_V2/TM_WorkSpace/

export RCUTILS_CONSOLE_OUTPUT_FORMAT="[{severity}] [{name}]: {message}"
source install/setup.bash
export ROS_DOMAIN_ID=1
#QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node tm_node_pkg tm_node

# Start tm_node in the background
ros2 run tm_node_pkg tm_node &
# Start gui_node with the QT framebuffer environment
QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node
