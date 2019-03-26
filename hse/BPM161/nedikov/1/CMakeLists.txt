cmake_minimum_required(VERSION 3.10)
project(parallel_hell)

set(CMAKE_CXX_STANDARD 14)

if (UNIX)
    set(CMAKE_CXX_FLAGS -pthread)
    add_definitions(-DUNIX)
endif()

add_executable(parallel_hell main.cpp)