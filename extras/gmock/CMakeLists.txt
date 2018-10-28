add_library(rapidcheck_gmock INTERFACE)
target_link_libraries(rapidcheck_gmock INTERFACE rapidcheck)
target_include_directories(rapidcheck_gmock INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# An INTERFACE library does not need to install anything but its headers
# and information on its targets.
install(TARGETS rapidcheck_gmock EXPORT rapidcheckConfig)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if (RC_ENABLE_TESTS)
  add_subdirectory(test)
endif()
