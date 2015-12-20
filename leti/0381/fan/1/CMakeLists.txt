# Minimum CMake required
cmake_minimum_required(VERSION 2.8)

# Project
project(chat)

set (CHAT_SERVER_NAME "chat_server")
set (CHAT_CLIENT_NAME "chat_client")

#add_definitions(-DGOOGLE_PROTOBUF_CMAKE_BUILD)
#set(BOOST_LIBRARYDIR /usr/lib/x86_64-linux-gnu/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_package(Boost COMPONENTS thread date_time program_options filesystem system REQUIRED)

if( NOT Boost_FOUND)
    message ( FATAL_ERROR "Boost not found")
else()
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIRS}
    )
endif()
if(DEBUG)
	set(PROTOBUFLIBNAME "libprotobufd.a")
else()
	set(PROTOBUFLIBNAME "libprotobuf.a")
endif()
if(G_PROTO_INCLUDE_DIR)
    
    message( STATUS "G_PROTO_INCLUDE_DIR=${G_PROTO_INCLUDE_DIR}")
    include_directories(${G_PROTO_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Undefined G_PROTO_INCLUDE_DIR env var. Use: -DG_PROTO_INCLUDE_DIR=<path to protobuf>")
endif()

if(NOT MESSAGE_PB_PATH)
    set( MESSAGE_PB_PATH ${CMAKE_SOURCE_DIR}/common/ )
endif()

message( STATUS "Set MESSAGE_PB_PATH to ${MESSAGE_PB_PATH}")
include_directories(${MESSAGE_PB_PATH})

if(G_PROTO_LIBRARY_DIR)
    
    message( STATUS "G_PROTO_LIBRARY_DIR = ${G_PROTO_LIBRARY_DIR}")
    link_directories(${G_PROTO_LIBRARY_DIR})
else()
    message(FATAL_ERROR "Undefined G_PROTO_LIBRARY_DIR env var. Use: -DG_PROTO_LIBRARY_DIR=<path to protobuf>")
endif()

include_directories(
	${CMAKE_SOURCE_DIR}/chat_client
	${CMAKE_SOURCE_DIR}/chat_server
	${CMAKE_SOURCE_DIR}/common
)

add_executable(${CHAT_SERVER_NAME}
	${CMAKE_SOURCE_DIR}/chat_server/chat_server
	${MESSAGE_PB_PATH}/Message.pb
)

target_link_libraries(${CHAT_SERVER_NAME}
	${PROTOBUFLIBNAME}
	${Boost_LIBRARIES}
)

add_executable(${CHAT_CLIENT_NAME}
	${CMAKE_SOURCE_DIR}/chat_client/chat_client
	${MESSAGE_PB_PATH}/Message.pb
)

target_link_libraries(${CHAT_CLIENT_NAME}
	${PROTOBUFLIBNAME}
	${Boost_LIBRARIES}
)

