cmake_minimum_required(VERSION 3.6)
project(gea_1)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp image_mat.h)
add_executable(gea_1 ${SOURCE_FILES})

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
message("=== USER PROVIDED PARAMETERS ===")
message("TBB_INCLUDE_DIR = '${TBB_INCLUDE_DIR}'")
message("TBB_LIBRARY = '${TBB_LIBRARY}'")
message("TBB_USE_DEBUG_BUILD = '${TBB_USE_DEBUG_BUILD}'")

find_package(TBB)
message("=== Find TBB Results ===")
message("TBB_LIBRARIES = '${TBB_LIBRARIES}'")
message("TBB_VERSION = '${TBB_VERSION}'")
message("TBB_INTERFACE_VERSION = '${TBB_INTERFACE_VERSION}'")
message("TBB_FOUND = '${TBB_FOUND}'")

include_directories(${TBB_INCLUDE_DIRS})
target_link_libraries(gea_1 ${TBB_LIBRARIES})

