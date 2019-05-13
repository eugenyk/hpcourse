cmake_minimum_required(VERSION 3.13)
project(producer-consumer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(producer-consumer main.cpp)