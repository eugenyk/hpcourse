cmake_minimum_required(VERSION 3.5)
project(lab03)

set(CMAKE_CXX_STANDARD 11)

list(APPEND
        CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    )

add_executable(lab03 src/main.cpp src/image.h src/elements.cpp src/elements.h src/common.h src/process.cpp src/process.h src/image.cpp)

find_package(TBB 4.4)
target_link_libraries(lab03 tbb)

# TODO choose OpenCV components.
find_package(OpenCV 3)

IF(OpenCV_FOUND)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DHAVE_OPENCV=1" )
    target_link_libraries(lab03 opencv_core opencv_highgui opencv_imgproc)
ENDIF(OpenCV_FOUND)
