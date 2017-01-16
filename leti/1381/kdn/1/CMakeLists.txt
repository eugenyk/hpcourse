cmake_minimum_required(VERSION 2.8)
project(flow_graph_lab1)

set(CMAKE_CXX_FLAGS "-std=c++0x")

add_executable(flow_graph_lab1 main.cpp image.cpp image.h)
target_link_libraries(flow_graph_lab1 tbb)
