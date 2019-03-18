cmake_minimum_required(VERSION 3.13)
project(producer-consumer)

set(CMAKE_CXX_STANDARD 14)

add_executable(producer-consumer main.cpp pthread_barrier.h)