cmake_minimum_required(VERSION 3.16)
project(hngac_hls_tb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(hngac_kernel
    src/hngac_kernel.cpp
)
target_include_directories(hngac_kernel PUBLIC include)
target_compile_options(hngac_kernel PRIVATE -O3 -Wall -Wextra -Wpedantic)

add_executable(hngac_kernel_tb
    tb/hngac_kernel_tb.cpp
)
target_link_libraries(hngac_kernel_tb PRIVATE hngac_kernel)
target_compile_options(hngac_kernel_tb PRIVATE -O3 -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME hngac_kernel_tb COMMAND hngac_kernel_tb)
