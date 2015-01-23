cmake_minimum_required(VERSION 2.8.12)
project(rapidcheck)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++11")

add_library(rapidcheck
  src/detail/AnsiEscape.cpp
  src/detail/Check.cpp
  src/gen/Generator.cpp
  src/detail/RandomEngine.cpp
  src/detail/RoseNode.cpp
  src/Show.cpp
  src/detail/Utility.cpp
  src/detail/ValueDescription.cpp
  src/detail/Results.cpp
  src/detail/Any.cpp
  src/detail/Assertions.cpp)

target_include_directories(rapidcheck PUBLIC include)

add_subdirectory(test)
add_subdirectory(examples)
