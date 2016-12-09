cmake_minimum_required(VERSION 3.6)
project(graph_task)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g")
set(SOURCE_FILES main.cc utils.h utils.cc graph.h graph.cc)
add_executable(graph_task ${SOURCE_FILES})
target_link_libraries(graph_task tbb)
