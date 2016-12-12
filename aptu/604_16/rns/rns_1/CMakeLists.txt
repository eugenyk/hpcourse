cmake_minimum_required(VERSION 3.0)

find_package(Boost COMPONENTS program_options REQUIRED)


find_library(TBB required)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_FLAGS "-g")
set (SOURCES main.cpp image.cpp image.h)

add_executable(lab_fg ${SOURCES})

include_directories(${TBB_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR})

target_link_libraries(lab_fg tbb ${Boost_LIBRARIES})