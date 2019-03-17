cmake_minimum_required(VERSION 3.13)
project(01_PosixThreads)

set(CMAKE_CXX_STANDARD 14)
add_executable(01_PosixThreads main.cpp
        barrier.h)