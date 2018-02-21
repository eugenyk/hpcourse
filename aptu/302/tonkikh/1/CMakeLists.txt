cmake_minimum_required(VERSION 3.9)
project(first_assignment C)

set(CMAKE_C_STANDARD 11)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -O2")
endif()
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_ENABLE")


find_package (Threads)

add_executable(first_assignment main.c)
target_link_libraries (first_assignment ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "a.out")
