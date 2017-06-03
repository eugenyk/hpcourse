cmake_minimum_required(VERSION 3.7)
project(03)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(SOURCE_FILES main.cpp)

add_executable(03 ${SOURCE_FILES})
target_link_libraries(03 tbb)

add_executable(gen_images gen_images.cpp)
