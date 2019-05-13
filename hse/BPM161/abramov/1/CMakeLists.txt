cmake_minimum_required(VERSION 3.0)
project(pthread-api)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        src/main.cpp)

include_directories(include)

find_package (Threads)
add_executable(pthread-api ${SOURCE_FILES})
target_link_libraries(pthread-api ${CMAKE_THREAD_LIBS_INIT})
