cmake_minimum_required(VERSION 3.6)
project(MultithreadingLab1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")



set(SOURCE_FILES main.cpp examples.cpp
        heder/Image.h
        heder/ImageSave.h
        heder/ImageInversion.h
        heder/ImageAverageBright.h heder/FindVectorPairIndexes.h heder/SetBorder.h heder/FindMaximum.h heder/FindMinimum.h heder/ImageGenerator.h)
add_executable(MultithreadingLab1 ${SOURCE_FILES}
        heder/Image.h
        heder/ImageSave.h
        heder/ImageInversion.h
        heder/ImageAverageBright.h heder/FindVectorPairIndexes.h heder/SetBorder.h heder/FindMaximum.h heder/FindMinimum.h heder/ImageGenerator.h)

target_link_libraries(MultithreadingLab1 tbb)
