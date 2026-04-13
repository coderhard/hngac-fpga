cmake_minimum_required(VERSION 3.16)
project(hngac_hls_tb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 QUIET)

add_library(hngac_kernel
    src/hngac_kernel.cpp
)
target_include_directories(hngac_kernel PUBLIC include)
target_compile_options(hngac_kernel PRIVATE -O3 -Wall -Wextra -Wpedantic)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(hngac_kernel PRIVATE -Wno-unknown-pragmas)
endif()

add_executable(hngac_kernel_tb
    tb/hngac_kernel_tb.cpp
)
target_link_libraries(hngac_kernel_tb PRIVATE hngac_kernel)
target_compile_options(hngac_kernel_tb PRIVATE -O3 -Wall -Wextra -Wpedantic)

add_executable(hngac_compare_benchmark
    bench/hngac_compare_benchmark.cpp
)
target_link_libraries(hngac_compare_benchmark PRIVATE hngac_kernel)
target_compile_options(hngac_compare_benchmark PRIVATE -O3 -Wall -Wextra -Wpedantic)
if(SQLite3_FOUND)
    target_link_libraries(hngac_compare_benchmark PRIVATE SQLite::SQLite3)
    target_compile_definitions(hngac_compare_benchmark PRIVATE HNGAC_HAVE_SQLITE=1)
endif()

enable_testing()
add_test(NAME hngac_kernel_tb COMMAND hngac_kernel_tb)
