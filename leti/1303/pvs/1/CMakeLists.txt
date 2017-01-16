cmake_minimum_required(VERSION 3.6)
project(FlowGraph)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules/)
set(TBB_ROOT_DIR libs/tbb)
find_package(TBB)

set(CMAKE_CXX_STANDARD 11)

#set(SOURCE_FILES main.cpp)
#set(SOURCE_FILES parallel_dct.cpp)
set(SOURCE_FILES src/dct2.cpp)
add_executable(FlowGraph ${SOURCE_FILES})

include_directories(${TBB_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
add_definitions(${TBB_DEFINITIONS})
target_link_libraries(FlowGraph ${TBB_LIBRARIES})