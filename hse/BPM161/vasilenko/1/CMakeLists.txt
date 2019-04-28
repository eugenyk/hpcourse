cmake_minimum_required(VERSION 3.13)
project(producer_consumer)
set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp)
add_executable(main ${SOURCE_FILES})
target_link_libraries(main LINK_PUBLIC pthread)
