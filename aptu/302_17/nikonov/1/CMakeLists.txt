cmake_minimum_required(VERSION 3.7)
project(hw01)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-pthread -Wall -Wextra -O2")

set(SOURCE_FILES main.cpp)
add_executable(hw01 ${SOURCE_FILES})