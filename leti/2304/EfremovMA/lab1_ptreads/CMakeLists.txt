cmake_minimum_required(VERSION 3.10)

project(hp_lab1_pthreads)

set (EXEC_NAME lab1_example)

set(SOURCE_FILES 
    main.cpp
    )

#set(PACKAGES
#    Threads
#    )

SET( CMAKE_CXX_FLAGS  "-pthread -g")

#set(LIBRARIES
#    ${CMAKE_THREAD_LIBS_INIT}
#    )

#find_package (${PACKAGES})

add_executable(${EXEC_NAME} ${SOURCE_FILES})

# target_link_libraries (${EXEC_NAME} ${LIBRARIES})

