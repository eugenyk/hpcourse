cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

find_package(Threads REQUIRED)

add_executable(producer_consumer main.cpp)
target_link_libraries(producer_consumer ${CMAKE_THREAD_LIBS_INIT})