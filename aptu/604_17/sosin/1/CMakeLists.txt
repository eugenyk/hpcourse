cmake_minimum_required(VERSION 3.8)
project(task01)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -pthread")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY})

set(SOURCE_FILES main.cpp)

add_executable(task01 ${SOURCE_FILES})
