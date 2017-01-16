cmake_minimum_required(VERSION 3.5.1)
project(flow_graph_imgproc)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

include_directories("/usr/local/include")
link_directories("/usr/lib/x86_64-linux-gnu")
set(CMAKE_CXX_FLAGS "-ltbb -std=c++0x ${CMAKE_CXX_FLAGS}")

add_executable(flow_graph_imgproc ${SOURCE_FILES})
target_link_libraries(flow_graph_imgproc tbb)
