cmake_minimum_required(VERSION 3.6)
project(UA_1)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(UA_1 ${SOURCE_FILES})
target_link_libraries(UA_1 tbb)