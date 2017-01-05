cmake_minimum_required (VERSION 2.8.8)
project (zda_1)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules/)

find_package(TBB)

include_directories("include")
include_directories("src")

set(SOURCES
    include/image.h
    include/maxbrightness.h
    include/minbrightness.h
    include/fixedbrightness.h
    include/pointhighlighter.h
    include/joinkeymatcher.h
    include/brightnessinverter.h
    include/meanbrightness.h
    include/fileoutput.h
    include/imagegenerator.h
    include/imagecleanup.h
    include/common.h
    src/image.cpp
    src/maxbrightness.cpp
    src/minbrightness.cpp
    src/fixedbrightness.cpp
    src/pointhighlighter.cpp
    src/joinkeymatcher.cpp
    src/brightnessinverter.cpp
    src/meanbrightness.cpp
    src/fileoutput.cpp
    src/imagegenerator.cpp
    src/imagecleanup.cpp
    src/main.cpp
    )

set(build_dir ${PROJECT_SOURCE_DIR}/build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${build_dir})
set(CMAKE_BUILD_FILES_DIRECTORY ${build_dir})
set(CMAKE_BUILD_DIRECTORY ${build_dir})
set(CMAKE_BINARY_DIR  ${build_dir})

include_directories(${TBB_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
add_definitions(${TBB_DEFINITIONS})
add_executable(zda_1 ${SOURCES})

target_link_libraries(zda_1 ${TBB_LIBRARIES})
