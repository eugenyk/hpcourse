cmake_minimum_required (VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)

project(flow-graph)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/flow-graph/cmake")

find_package(TBB REQUIRED)

add_definitions(-Wall -Wextra -pedantic -g)

add_executable(flow-graph src/main.cpp src/ImageProcessor.cpp src/ImageProcessor.h src/Image.cpp src/Image.h)
target_include_directories (flow-graph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(flow-graph tbb)