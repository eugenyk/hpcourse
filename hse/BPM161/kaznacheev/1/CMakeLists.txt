cmake_minimum_required(VERSION 3.13)
project(parallel)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)

find_package (Threads)

add_executable(parallel main.cpp)
