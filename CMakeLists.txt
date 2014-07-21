cmake_minimum_required(VERSION 2.8.12)
project(rapidcheck)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++11")

add_library(rapidcheck
  src/AnsiEscape.cpp
  src/Check.cpp
  src/Generator.cpp
  src/RandomEngine.cpp
  src/Rose.cpp
  src/Show.cpp
  src/Utility.cpp
  src/ValueDescription.cpp
  src/Results.cpp)

target_include_directories(rapidcheck PUBLIC include)

add_subdirectory(test)
add_subdirectory(examples)
