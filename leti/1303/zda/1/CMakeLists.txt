cmake_minimum_required (VERSION 2.8.8)
project (zda_1)

# Add the project modules directory to the CMake modules search path.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules/)

# Find TBB libraries and header files
find_package(TBB)

# Add include dirctory and compile definitions for all targets
include_directories(${TBB_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
add_definitions(${TBB_DEFINITIONS})
add_executable(my_exec main.cpp)

# Link your target with TBB libraries
target_link_libraries(my_exec ${TBB_LIBRARIES})
