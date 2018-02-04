cmake_minimum_required(VERSION 2.8)

project(Work1)
add_executable(${PROJECT_NAME} "main.cpp")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread)
