cmake_minimum_required(VERSION 3.8)
project(ngac_auth)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

include_directories(include)

add_executable(auth_node src/auth_node.cpp)
ament_target_dependencies(auth_node rclcpp std_msgs geometry_msgs)

add_executable(bad_actor_node src/bad_actor_node.cpp)
ament_target_dependencies(bad_actor_node rclcpp std_msgs)

install(TARGETS auth_node bad_actor_node DESTINATION lib/${PROJECT_NAME})
ament_package()
