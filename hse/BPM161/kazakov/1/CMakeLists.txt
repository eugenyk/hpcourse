cmake_minimum_required(VERSION 3.13)
project(untitled)

find_package(Threads)
set(CMAKE_CXX_STANDARD 14)

add_executable(lab1 src/main.cpp include/consumer_parameters.h include/random.h src/random.cpp src/value_storage.cpp include/value_storage.h include/no_value_exception.h)
target_link_libraries(lab1 ${CMAKE_THREAD_LIBS_INIT})