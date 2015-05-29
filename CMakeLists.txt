cmake_minimum_required(VERSION 2.8.12)
project(rapidcheck)
enable_testing()

option(RC_BUILD_TESTS "Build RapidCheck tests" OFF)
option(RC_BUILD_EXAMPLES "Build RapidCheck examples" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++11")

add_library(rapidcheck
  src/Check.cpp
  src/Classify.cpp
  src/GenerationFailure.cpp
  src/Random.cpp
  src/Show.cpp
  src/detail/Any.cpp
  src/detail/Assertions.cpp
  src/detail/Configuration.cpp
  src/detail/ExpressionCaptor.cpp
  src/detail/FrequencyMap.cpp
  src/detail/ImplicitParam.cpp
  src/detail/MapParser.cpp
  src/detail/Property.cpp
  src/detail/PropertyContext.cpp
  src/detail/Results.cpp
  src/detail/TestParams.cpp
  src/detail/Utility.cpp
  src/gen/Numeric.cpp
  src/gen/Text.cpp
  src/gen/detail/ExecHandler.cpp
  src/gen/detail/GenerationHandler.cpp
  src/gen/detail/Recipe.cpp
  src/gen/detail/ScaleInteger.cpp
  )

# Random is used a LOT so it should preferrably be really fast
set_property(SOURCE src/Random.cpp
  APPEND_STRING PROPERTY COMPILE_FLAGS " -O3")

target_include_directories(rapidcheck PUBLIC include)

add_subdirectory(ext)

if (RC_BUILD_TESTS)
  add_subdirectory(test)
endif()

if (RC_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

add_subdirectory(extras)