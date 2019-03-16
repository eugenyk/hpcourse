cmake_minimum_required(VERSION 3.12)
project(producerconsumer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(producerconsumer main.cpp)
