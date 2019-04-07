cmake_minimum_required(VERSION 3.10)
project(producer-consumer)

set(CMAKE_CXX_STANDARD 14)

add_executable(producer-consumer main.cpp)

find_package(Threads)
target_link_libraries (producer-consumer ${CMAKE_THREAD_LIBS_INIT})
