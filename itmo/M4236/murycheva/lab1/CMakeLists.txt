cmake_minimum_required(VERSION 3.9)
project(hpc_hw1)

find_library(PTHREAD_LIBRARY pthread)

set(CMAKE_CXX_STANDARD 11)

add_executable(hpc_hw1 main.cpp)

target_link_libraries(hpc_hw1 ${PTHREAD_LIBRARY})