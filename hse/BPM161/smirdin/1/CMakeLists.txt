cmake_minimum_required(VERSION 3.13)
project(Parallel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-pthread -Werror -Wall -pedantic")

add_executable(Parallel main.cpp)