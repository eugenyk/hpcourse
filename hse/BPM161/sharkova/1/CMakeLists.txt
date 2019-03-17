cmake_minimum_required(VERSION 3.10)
project(producerconsumer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(producerconsumer main.cpp)
