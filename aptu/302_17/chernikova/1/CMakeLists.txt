cmake_minimum_required(VERSION 3.3)
project(scaffolder)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pthread")

add_executable(main main.cpp)
