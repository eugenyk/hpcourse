cmake_minimum_required(VERSION 2.8)

project(sia_1)

set(CMAKE_CXX_STANDARD 11)

add_executable(sia_1 main.cpp)

find_library(TBB required)

include_directories(${TBB_INCLUDE_DIRS})

target_link_libraries(sia_1 tbb)