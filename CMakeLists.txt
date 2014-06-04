cmake_minimum_required(VERSION 2.8)
project(rapidcheck)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++11")

add_library(rapidcheck
  src/AnsiEscape.cpp
  src/Check.cpp
  src/ConsoleDelegate.cpp
  src/Generator.cpp
  src/RandomEngine.cpp
  src/Rose.cpp
  src/Show.cpp
  src/Suite.cpp
  src/Utility.cpp)

target_include_directories(rapidcheck PUBLIC include)

add_subdirectory(test)
