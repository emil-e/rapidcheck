project(rapidcheck)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
add_executable(rapidcheck main.cpp)
target_include_directories(rapidcheck PRIVATE .)