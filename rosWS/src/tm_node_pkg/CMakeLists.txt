cmake_minimum_required(VERSION 3.8)
project(tm_node_pkg)

set(TM_V2_DIR "$ENV{HOME}/git_sasan/ICA_TM_V2/TM_V2")
set(CMAKE_BUILD_RPATH "${TM_V2_DIR}")
set(CMAKE_INSTALL_RPATH "${TM_V2_DIR}")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(my_msgs_pkg REQUIRED)

add_executable(tm_node src/tm_node.cpp)

target_include_directories(tm_node PRIVATE
  "${TM_V2_DIR}/Source"
)
target_link_libraries(tm_node
  #ICA_USV_Test  # Drops the 'lib' and '.a' automatically
  "${TM_V2_DIR}/libICA_USV_Test.so"
  qrencode       # 🧠 Add this line
  gd             # 🧠 And this one
)



ament_target_dependencies(tm_node rclcpp std_msgs my_msgs_pkg)

install(TARGETS
  tm_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
