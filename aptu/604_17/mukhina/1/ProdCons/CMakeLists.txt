cmake_minimum_required(VERSION 3.7)
project(ProdCons)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
add_executable(ProdCons ${SOURCE_FILES})