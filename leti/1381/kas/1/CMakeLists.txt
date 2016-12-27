cmake_minimum_required(VERSION 3.2.0)

project(kas_1)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_EXE main.cpp Image.h Image.cpp Options.h Options.cpp)

add_executable(kas_1 ${SOURCE_EXE})

target_link_libraries(kas_1 tbb)
