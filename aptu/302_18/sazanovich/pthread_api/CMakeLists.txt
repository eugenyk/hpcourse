cmake_minimum_required(VERSION 3.9)
project(pthread_api)

set(CMAKE_CXX_STANDARD 14)

add_executable(pthread_api main.cpp)
target_link_libraries(pthread_api pthread)
