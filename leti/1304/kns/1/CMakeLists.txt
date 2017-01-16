cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(tbb_lab)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp flow.cpp flow.h utils.h utils.cpp)
add_executable(tbb_lab ${SOURCE_FILES})

find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})
target_link_libraries(tbb_lab ${ImageMagick_LIBRARIES})

add_subdirectory(external/tbb)
target_link_libraries(tbb_lab tbb)