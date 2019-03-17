set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -Wall -Wextra -pedantic -DLOCAL")

add_executable(lab1 lab1.cpp)
add_executable(gen_data gen_data.cpp)
