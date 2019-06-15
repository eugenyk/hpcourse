cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

add_executable(pthread_api main.cpp)
target_link_libraries(pthread_api LINK_PUBLIC pthread)
