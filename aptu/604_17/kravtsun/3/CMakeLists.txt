cmake_minimum_required(VERSION 3.5)
project(lab03)

set(CMAKE_CXX_STANDARD 11)

list(APPEND
        CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    )

find_package(
    TBB # COMPONENTS tbbmalloc tbbmalloc_proxy tbb_preview
    )

add_executable(lab03 src/main.cpp src/image.h src/elements.cpp src/elements.h)
target_link_libraries(lab03 tbb)

