project(flow_graph_lab)
cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)

aux_source_directory(. SRC_LIST)
include_directories(include)


add_executable(${PROJECT_NAME} "main.cpp")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} tbb pthread X11)

add_custom_command(
 TARGET ${PROJECT_NAME} POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_SOURCE_DIR}/imgs" "${CMAKE_CURRENT_BINARY_DIR}/imgs"
)
