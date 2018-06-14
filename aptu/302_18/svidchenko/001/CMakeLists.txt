# cmake_minimum_required(VERSION <specify CMake version here>)
project(001)

set(CMAKE_CXX_STANDARD 14)

find_package (Threads)

add_executable(001 main.cpp)
target_link_libraries (001 ${CMAKE_THREAD_LIBS_INIT})

