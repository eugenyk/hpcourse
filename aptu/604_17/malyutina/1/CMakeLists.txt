cmake_minimum_required(VERSION 3.8)
project(main)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")


set(SOURCE_FILES main.cpp)
add_executable(main ${SOURCE_FILES})
