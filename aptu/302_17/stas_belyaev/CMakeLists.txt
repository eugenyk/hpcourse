cmake_minimum_required(VERSION 3.7)
project(ParallelProgramming)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SOURCE_FILES main.cpp)
add_executable(ParallelProgramming ${SOURCE_FILES})