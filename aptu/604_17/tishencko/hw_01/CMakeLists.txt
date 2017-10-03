cmake_minimum_required(VERSION 3.5)
project(hw_01)

set(CMAKE_CXX_FLAGS -pthread)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(hw_01 ${SOURCE_FILES})
