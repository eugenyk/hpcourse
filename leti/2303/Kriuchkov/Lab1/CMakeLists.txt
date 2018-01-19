cmake_minimum_required(VERSION 3.7)
 project(1)
 
 set(CMAKE_CXX_STANDARD 11)
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
 
 set(SOURCE_FILES main.cpp)
 add_executable(1 ${SOURCE_FILES})