cmake_minimum_required(VERSION 3.5)

project(create3_description)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(irobot_create_msgs REQUIRED)

add_executable(dynamic_joint_state_publisher src/dynamic_joint_state_publisher.cpp)
ament_target_dependencies(dynamic_joint_state_publisher
  rclcpp
  sensor_msgs
  nav_msgs
  irobot_create_msgs
)

install(TARGETS
  dynamic_joint_state_publisher
  DESTINATION lib/${PROJECT_NAME})

install(
  DIRECTORY
    meshes
    urdf
  DESTINATION
    share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
