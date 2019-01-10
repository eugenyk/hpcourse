cmake_minimum_required(VERSION 3.9)
project(lock_free_queue)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/inc)
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(lock_free_queue main.cpp queue.cpp queue.h)
target_link_libraries(lock_free_queue cds pthread)


