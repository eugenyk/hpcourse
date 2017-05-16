cmake_minimum_required(VERSION 3.6)
project(03)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(03 ${SOURCE_FILES})
target_link_libraries(03 tbb)