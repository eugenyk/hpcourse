cmake_minimum_required(VERSION 3.13)
project(lab1)

find_package(Threads)
set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(lab1
        include/consumer_parameters.h
        include/no_value_exception.h
        include/random.h
        include/value_storage.h
        src/main.cpp
        src/random.cpp
        src/value_storage.cpp
        CMakeLists.txt)
target_link_libraries(lab1 ${CMAKE_THREAD_LIBS_INIT})
