cmake_minimum_required(VERSION 2.8)
project(dip_1)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(dip_1 ${SOURCE_FILES})

find_library(TBB required)

include_directories(${TBB_INCLUDE_DIRS})
target_link_libraries(dip_1 tbb)
