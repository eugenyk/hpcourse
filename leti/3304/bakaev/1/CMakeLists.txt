cmake_minimum_required(VERSION 2.8)

project(producer_and_consumers)

add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} pthread)
