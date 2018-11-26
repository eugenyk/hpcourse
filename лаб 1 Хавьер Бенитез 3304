cmake_minimum_required(VERSION 3.5.1)
project(lab1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Werror")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(1 main.cpp)
target_link_libraries(1 Threads::Threads)
