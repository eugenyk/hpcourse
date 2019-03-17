

cmake_minimum_required(VERSION 3.13)
project(lab1)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread -std=c++17")

add_executable(lab1 main.cpp)

target_link_libraries(${PROJECT_NAME} pthread)