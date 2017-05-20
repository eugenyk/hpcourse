cmake_minimum_required(VERSION 3.6)
project(algo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

include_directories(/usr/local/Cellar/tbb/2017_U6/include)
file(GLOB LIBRARIES "/usr/local/Cellar/tbb/2017_U6/lib/*.dylib")

set(CMAKE_C_FLAGS “${CMAKE_C_FLAGS} -std=c++11 -ltbb”)
set(SOURCE_FILES main.cpp)

add_executable(algo ${SOURCE_FILES})
target_link_libraries(algo ${LIBRARIES})