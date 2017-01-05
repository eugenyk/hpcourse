cmake_minimum_required(VERSION 2.8)
project(lab_№1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")



set(SOURCE_FILES main.cpp
        heder/Image.h
        heder/ImageSave.h
        heder/ImageInversion.h
        heder/ImageAverageBright.h heder/FindVectorPairIndexes.h heder/SetBorder.h heder/FindMaximum.h heder/FindMinimum.h heder/ImageGenerator.h)
add_executable(lab_№1 ${SOURCE_FILES}
        heder/Image.h
        heder/ImageSave.h
        heder/ImageInversion.h
        heder/ImageAverageBright.h heder/FindVectorPairIndexes.h heder/SetBorder.h heder/FindMaximum.h heder/FindMinimum.h heder/ImageGenerator.h)

target_link_libraries(lab_№1 tbb)
