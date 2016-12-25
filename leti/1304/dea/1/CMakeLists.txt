cmake_minimum_required(VERSION 2.8)
project(FlowGraph_DeminAD)

set(CMAKE_CXX_FLAGS "-std=c++0x")

set(HEADER_FILES image.h pixel.h program_settings.h)
set(SOURCE_FILES main.cpp image.cpp program_settings.cpp)
add_executable(FlowGraph_DeminAD ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(FlowGraph_DeminAD tbb)
