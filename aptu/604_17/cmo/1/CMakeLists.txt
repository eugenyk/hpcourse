cmake_minimum_required(VERSION 2.8)
project(pthread_api)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES src/main.cpp)
add_executable(pthread_api ${SOURCE_FILES})
