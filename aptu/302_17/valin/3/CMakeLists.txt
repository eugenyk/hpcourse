cmake_minimum_required(VERSION 3.7)
project(img_tbb)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(img_tbb ${SOURCE_FILES})
target_link_libraries(img_tbb tbb)