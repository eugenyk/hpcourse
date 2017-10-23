cmake_minimum_required(VERSION 3.8)
project(lab1)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
add_executable(lab1 ${SOURCE_FILES})