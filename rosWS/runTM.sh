#!/bin/bash
cd ~/git_sasan/ICA_TM_V2/rosWS
export TM_WORKSPACE=~/git_sasan/TM_WorkSpace/
export RCUTILS_CONSOLE_OUTPUT_FORMAT="[{severity}] [{name}]: {message}"
source install/setup.bash
export ROS_DOMAIN_ID=1
#QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node tm_node_pkg tm_node

# Start tm_node in the background
ros2 run tm_node_pkg tm_node &
# Start gui_node with the QT framebuffer environment
QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node
