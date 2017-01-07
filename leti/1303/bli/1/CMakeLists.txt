cmake_minimum_required(VERSION 2.8)
project(lab1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

include_directories(~/tbb2017_20161004oss/include)
LINK_DIRECTORIES(~/tbb2017_20161004oss/build/linux_intel64_gcc_cc5.4.0_libc2.23_kernel4.4.0_debug)

add_executable(main main.cpp)
TARGET_LINK_LIBRARIES(main tbb_debug)
