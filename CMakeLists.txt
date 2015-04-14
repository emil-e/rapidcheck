cmake_minimum_required(VERSION 2.8.12)
project(rapidcheck)
enable_testing()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++11 -ftemplate-backtrace-limit=0")

add_library(rapidcheck
  src/GenerationFailure.cpp
  src/Random.cpp
  src/Show.cpp
  src/detail/Any.cpp
  src/detail/Assertions.cpp
  src/detail/Check.cpp
  src/detail/Configuration.cpp
  src/detail/ExpressionCaptor.cpp
  src/detail/ImplicitParam.cpp
  src/detail/MapParser.cpp
  src/detail/Property.cpp
  src/detail/Results.cpp
  src/detail/TestParams.cpp
  src/detail/Utility.cpp
  src/gen/Text.cpp
  src/gen/detail/ExecHandler.cpp
  src/gen/detail/GenerationHandler.cpp
  src/gen/detail/Recipe.cpp
  )

target_include_directories(rapidcheck PUBLIC include)

add_subdirectory(test)
add_subdirectory(examples)
