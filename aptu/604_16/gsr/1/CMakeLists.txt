cmake_minimum_required(VERSION 3.6)
project(gsr_1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_path(TBB_INCLUDE_DIR tbb)
find_library(TBB_LIBRARY libtbb)
include_directories(${TBB_INCLUDE_DIR})

set(SOURCE_FILES main.cpp img_ops.h)
add_executable(gsr_1 ${SOURCE_FILES})
target_link_libraries(gsr_1 tbb)