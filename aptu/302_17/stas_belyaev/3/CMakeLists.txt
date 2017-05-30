cmake_minimum_required(VERSION 3.7)
project(3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

add_executable(3 ${SOURCE_FILES})
file(GLOB LIBRARIES "/usr/local/Cellar/tbb/2017_U6/lib/*.dylib")
target_link_libraries(3 ${LIBRARIES})
