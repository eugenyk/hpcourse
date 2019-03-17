cmake_minimum_required(VERSION 3.13)
project(lab_1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(lab_1 main.cpp)