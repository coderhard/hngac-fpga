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

# Path to the HLS kernel source and headers (relative to this CMakeLists.txt).
set(HNGAC_HLS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../fpga/hls")

# --- DCAS baseline nodes (unchanged) ---

add_executable(auth_node src/auth_node.cpp)
ament_target_dependencies(auth_node rclcpp std_msgs geometry_msgs)

add_executable(bad_actor_node src/bad_actor_node.cpp)
ament_target_dependencies(bad_actor_node rclcpp std_msgs)

# --- IPCCC 5D adversarial demo nodes ---

# 5D gatekeeper: compiles hngac_kernel.cpp directly to avoid a separate
# find_package step. HLS pragmas are suppressed with -Wno-unknown-pragmas.
add_executable(gatekeeper_5d_node
    src/gatekeeper_5d_node.cpp
    ${HNGAC_HLS_ROOT}/src/hngac_kernel.cpp
)
target_include_directories(gatekeeper_5d_node PRIVATE
    ${HNGAC_HLS_ROOT}/include
)
target_compile_options(gatekeeper_5d_node PRIVATE
    -O3 -Wno-unknown-pragmas
)
ament_target_dependencies(gatekeeper_5d_node rclcpp std_msgs geometry_msgs)

# Compromised authenticated node: Subject 1, wrong provenance. No kernel dep.
add_executable(compromised_ros2_node src/compromised_ros2_node.cpp)
ament_target_dependencies(compromised_ros2_node rclcpp std_msgs)

# Legitimate node: Subject 1, correct provenance. No kernel dep.
add_executable(legit_ros2_node src/legit_ros2_node.cpp)
ament_target_dependencies(legit_ros2_node rclcpp std_msgs)

install(TARGETS
    auth_node
    bad_actor_node
    gatekeeper_5d_node
    compromised_ros2_node
    legit_ros2_node
    DESTINATION lib/${PROJECT_NAME}
)

ament_package()
