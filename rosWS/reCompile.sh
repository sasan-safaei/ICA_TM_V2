#!/bin/bash
rm -rf build/ log/ install/

colcon build --packages-select my_msgs_pkg
source install/setup.bash
colcon build
#colcon build --packages-select qt_gui_node_pkg
#colcon build --packages-select tm_node_pkg
source install/setup.bash
