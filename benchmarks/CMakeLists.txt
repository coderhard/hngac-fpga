cmake_minimum_required(VERSION 3.5)
project(robo_ngac_benchmarks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The Throughput Test (Fastest Average)
add_executable(ngac_benchmark ngac_benchmark.cpp)
target_compile_options(ngac_benchmark PRIVATE -O3 -Wall -Wextra)

# The Jitter Test (Detailed Distribution)
add_executable(ngac_jitter ngac_jitter.cpp)
target_compile_options(ngac_jitter PRIVATE -O3 -Wall -Wextra)
