cmake_minimum_required(VERSION 3.5)
project(1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -pthread")
add_executable(1 main.cpp)