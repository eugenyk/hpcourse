cmake_minimum_required(VERSION 3.7)
project(TBB)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES project/main.cpp)
set(GEN_FILES generation_images/gen_images.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltbb")

add_executable(TBB ${SOURCE_FILES})
add_executable(gen_images ${GEN_FILES})