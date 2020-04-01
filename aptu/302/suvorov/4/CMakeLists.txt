cmake_minimum_required(VERSION 3.0)

find_package(TBB REQUIRED tbb)

add_executable(hw04 src/main.cpp)
target_link_libraries(hw04 TBB::tbb)
