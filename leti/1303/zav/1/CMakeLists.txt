cmake_minimum_required(VERSION 2.8)
project(tbb_lab1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -ltbb_debug")
set(SOURCES main.cpp matrix.cpp matrix.h test.cpp test.h matrix_algorithm.cpp matrix_algorithm.h)

#include_directories(include ~/Downloads/tbb2017_20161004oss/include)
#LINK_DIRECTORIES(/home/sasha/Downloads/tbb2017_20161004oss/build/linux_ia32_gcc_cc5.4.0_libc2.23_kernel4.4.0_debug)
add_executable(main ${SOURCES})
TARGET_LINK_LIBRARIES(main tbb_debug)
