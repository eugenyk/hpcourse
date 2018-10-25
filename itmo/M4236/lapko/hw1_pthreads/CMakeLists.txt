cmake_minimum_required(VERSION 3.12)
project(hw1_pthreads)
set(CMAKE_CXX_STANDARD 14)

#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Werror -O0 -g")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -O0 -g")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(hw1_pthreads main.cpp)
target_link_libraries(hw1_pthreads Threads::Threads)