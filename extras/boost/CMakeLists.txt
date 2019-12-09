add_library(rapidcheck_boost INTERFACE)
target_link_libraries(rapidcheck_boost INTERFACE rapidcheck)
target_include_directories(rapidcheck_boost INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# An INTERFACE library does not need to install anything but its headers
# and information on its targets.
install(TARGETS rapidcheck_boost EXPORT rapidcheckConfig)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if (RC_ENABLE_TESTS)
  add_subdirectory(test)
endif()
