cmake_minimum_required(VERSION 3.5.1)

project(hpclab1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(HEADERS headers/CImage.h headers/CAverage.h headers/CImageGen.h headers/CInversion.h headers/CSaveAverage.h headers/Maximum.h 
            headers/Minimum.h headers/MinMaxIndexesVector.h headers/SetFrame.h)

set(SOURCES main.cpp)


add_definitions(-Wall )

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} )

target_link_libraries(${PROJECT_NAME} tbb)

