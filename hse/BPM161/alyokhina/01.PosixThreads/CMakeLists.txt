cmake_minimum_required(VERSION 3.5.1)
project(01_PosixThreads)

set(CMAKE_CXX_STANDARD 11)

add_executable(01_PosixThreads
        main.cpp)
find_package(Threads)
target_link_libraries (01_PosixThreads ${CMAKE_THREAD_LIBS_INIT})