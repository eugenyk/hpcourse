cmake_minimum_required(VERSION 3.13)
project(HW1_Pthread)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(HW1_Pthread main.cpp)