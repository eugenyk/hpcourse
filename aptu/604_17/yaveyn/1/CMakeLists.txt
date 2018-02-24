cmake_minimum_required(VERSION 3.0.0)
project(parallel-lab1 VERSION 0.0.0)

include(CTest)
enable_testing()


find_package(Threads)
add_executable(parallel-lab1 main.cpp)

target_link_libraries(parallel-lab1 ${CMAKE_THREAD_LIBS_INIT})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
