cmake_minimum_required(VERSION 2.8)
project(flow_graph)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(flow_graph ${SOURCE_FILES})

find_library(TBB required)

include_directories(${TBB_INCLUDE_DIRS})
target_link_libraries(flow_graph tbb)
