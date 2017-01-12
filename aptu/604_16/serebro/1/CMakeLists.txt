cmake_minimum_required(VERSION 3.4)
project(lab2)

set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Boost REQUIRED COMPONENTS program_options)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)

add_executable(lab2 ${SOURCE_FILES})
#assuming tbb is in standard include and library path
target_link_libraries(lab2 tbb  ${Boost_LIBRARIES})