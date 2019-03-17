cmake_minimum_required(VERSION 3.5)
project(homework01)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -lpthread)

include_directories(.)

add_executable(homework01 main.cpp)

