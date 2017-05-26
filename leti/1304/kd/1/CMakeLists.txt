# Minimum CMake required
cmake_minimum_required(VERSION 2.8)

# Project
project(TBBImages)

set (PROGRAM_NAME "tbb_images")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ltbb")

set(CPP_SOURCES
        src/main.cpp
        src/AverageBrightnessCounter.cpp
        src/FinderOnImage.cpp
        src/Image.cpp
        src/ImageFiller.cpp
        src/ImageGenerator.cpp
        src/ImageHighlighter.cpp
        src/ImageInverser.cpp
        src/ImageTransformer.cpp
        src/main.cpp)

include_directories(
  ${CMAKE_SOURCE_DIR}/include
)

add_executable(${PROGRAM_NAME}
  ${CPP_SOURCES}
)