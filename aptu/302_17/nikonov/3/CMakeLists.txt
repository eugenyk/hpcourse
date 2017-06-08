cmake_minimum_required(VERSION 3.7)
project(hw03)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp Picture.cpp Picture.h Options.cpp Options.h)

add_executable(hw03 ${SOURCE_FILES})
target_link_libraries(hw03 tbb)
