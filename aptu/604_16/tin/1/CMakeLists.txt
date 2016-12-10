cmake_minimum_required(VERSION 3.5)
project(flow_graph)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

list(APPEND CMAKE_MODULE_PATH "FindTBB.cmake")

find_package(TBB COMPONENTS tbbmalloc tbbmalloc_proxy tbb_preview)

set(SOURCE_FILES main.cpp image.cpp)
add_executable(flow_graph ${SOURCE_FILES})
target_link_libraries(flow_graph tbb)