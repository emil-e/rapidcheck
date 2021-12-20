cmake_minimum_required(VERSION 3.0)
project(rapidcheck CXX)

# Don't warn about symbol visibility for static libraries with CMake 3.3 and later.
if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 11)

option(RC_ENABLE_TESTS "Build RapidCheck tests" OFF)
option(RC_ENABLE_EXAMPLES "Build RapidCheck examples" OFF)
option(RC_ENABLE_RTTI "Build RapidCheck with RTTI" ON)

if(MSVC)
  # /bigobj - some object files become very large so we need this
  # /wd4503 - truncation of decorated name, not much we can do about it so
  #           disable it
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /wd4503 /WX")
  # /RTC* is incompatible with /O2 needed for Random.cpp to speed it up
  string(REGEX REPLACE "/RTC(su|[1su])" ""
    CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces -Wno-unused-command-line-argument")
  if(APPLE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)")
      # Workaround for legacy Apple environment (cf. https://stackoverflow.com/a/19774902/1682144)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
  endif()
endif()

# RapidCheck will be built either as a static or a dynamic library depending on the CMake global
# variable BUILD_SHARED_LIBS (https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html).
# If you wish to change the library type of RapidCheck, you can either specify the variable when invoking CMake
# or, if you are including RapidCheck as a subdirectory and wish to control the library type, you can set
# the variable before the call to add_subdirectory and reset it afterwards, if necessary.
add_library(rapidcheck
  src/BeforeMinimalTestCase.cpp
  src/Check.cpp
  src/Classify.cpp
  src/GenerationFailure.cpp
  src/Log.cpp
  src/Random.cpp
  src/Show.cpp
  src/detail/Any.cpp
  src/detail/Assertions.cpp
  src/detail/Base64.cpp
  src/detail/Configuration.cpp
  src/detail/DefaultTestListener.cpp
  src/detail/FrequencyMap.cpp
  src/detail/ImplicitParam.cpp
  src/detail/LogTestListener.cpp
  src/detail/MapParser.cpp
  src/detail/MulticastTestListener.cpp
  src/detail/ParseException.cpp
  src/detail/Platform.cpp
  src/detail/Property.cpp
  src/detail/PropertyContext.cpp
  src/detail/ReproduceListener.cpp
  src/detail/Results.cpp
  src/detail/Serialization.cpp
  src/detail/StringSerialization.cpp
  src/detail/TestMetadata.cpp
  src/detail/TestParams.cpp
  src/detail/Testing.cpp
  src/gen/Numeric.cpp
  src/gen/Text.cpp
  src/gen/detail/ExecHandler.cpp
  src/gen/detail/GenerationHandler.cpp
  src/gen/detail/Recipe.cpp
  src/gen/detail/ScaleInteger.cpp
  )

# Random is used a LOT so it should preferably be really fast.
if(MSVC)
  set_property(SOURCE src/Random.cpp
    APPEND_STRING PROPERTY COMPILE_FLAGS " /O2")
else()
  set_property(SOURCE src/Random.cpp
    APPEND_STRING PROPERTY COMPILE_FLAGS " -O3")
endif()

target_include_directories(rapidcheck PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
)

include(GNUInstallDirs)
install(TARGETS rapidcheck EXPORT rapidcheckConfig
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # This is for Windows
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# On Windows under MinGW, random_device provides no entropy,
# so it will always return the same value.
# Seed using system time instead.
# See: https://stackoverflow.com/questions/18880654/why-do-i-get-the-same-sequence-for-every-run-with-stdrandom-device-with-mingw
if(MINGW)
  target_compile_definitions(rapidcheck PRIVATE RC_SEED_SYSTEM_TIME)
endif()

if(NOT RC_ENABLE_RTTI)
  target_compile_definitions(rapidcheck PUBLIC RC_DONT_USE_RTTI)
endif()

add_subdirectory(ext)

if(RC_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(RC_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

add_subdirectory(extras)

# Install the export file specifying all the targets for RapidCheck
install(EXPORT rapidcheckConfig DESTINATION share/rapidcheck/cmake)
export(EXPORT rapidcheckConfig FILE rapidcheckConfig.cmake)
