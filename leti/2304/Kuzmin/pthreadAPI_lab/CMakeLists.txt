cmake_minimum_required(VERSION 2.8)

project(pthreadAPI_lab)
add_executable(${PROJECT_NAME} "main.cpp")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread)
