cmake_minimum_required(VERSION 2.8)
project(posix)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_executable(posix main.cpp value.h)
