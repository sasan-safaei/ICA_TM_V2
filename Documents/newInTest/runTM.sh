#!/bin/bash
# Clear the screen
#clear

# Redirect stdout and stderr to null
#exec >/dev/null 2>&1

#chvt 2

echo -e "\e[?25l"
clear

#sudo systemctl disable getty@tty1.service


cd ~/ica_usv_testdevice-gui/rosWS
source install/setup.bash
export ROS_DOMAIN_ID=1
#QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node tm_node_pkg tm_node

# Start tm_node in the background
ros2 run tm_node_pkg tm_node &
# Start gui_node with the QT framebuffer environment
QT_QPA_PLATFORM=linuxfb ros2 run qt_gui_node_pkg gui_node
