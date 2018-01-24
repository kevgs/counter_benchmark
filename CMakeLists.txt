cmake_minimum_required(VERSION 3.0)

project (counter CXX)

add_library(counter INTERFACE)
target_include_directories(counter INTERFACE .)

add_executable(counter-benchmark counter_benchmark.cc)
target_include_directories(counter-benchmark PRIVATE benchmark/include)
target_link_libraries(counter-benchmark PRIVATE benchmark)

add_subdirectory(benchmark)
