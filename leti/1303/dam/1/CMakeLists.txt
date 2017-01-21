cmake_minimum_required(VERSION 3.5.1)

project(hpclab1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(HEADERS heads/CImage.h heads/CAverage.h heads/CImageGen.h heads/CInversion.h heads/CSaveAverage.h heads/Maximum.h 
            heads/Minimum.h heads/MinMaxIndexesVector.h heads/SetFrame.h)

set(SOURCES main.cpp)


add_definitions(-Wall )

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} )

target_link_libraries(${PROJECT_NAME} tbb)

