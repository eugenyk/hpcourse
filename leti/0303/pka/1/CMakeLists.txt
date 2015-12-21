cmake_minimum_required(VERSION 3.1)
project(Chat)

set( CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS system program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders Message.proto)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_BINARY_DIR})

FIND_PACKAGE (Threads REQUIRED)

set(SRC_LIST src/chat.cpp src/client.cpp src/message.cpp src/server.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${ProtoSources})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
