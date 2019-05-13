cmake_minimum_required(VERSION 3.13)
project(posix_threads_task)

set(CMAKE_CXX_STANDARD 14)

add_executable(posix_threads_task main.cpp)
target_link_libraries(${PROJECT_NAME} pthread)