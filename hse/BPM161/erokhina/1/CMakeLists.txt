cmake_minimum_required(VERSION 3.10)
project(lab1)

find_library(PTHREAD_LIBRARY pthread)

set(CMAKE_CXX_STANDARD 11)

add_executable(lab1 main.cpp)

target_link_libraries(lab1 ${PTHREAD_LIBRARY})