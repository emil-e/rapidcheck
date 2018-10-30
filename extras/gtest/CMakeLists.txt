add_library(rapidcheck_gtest INTERFACE)
target_link_libraries(rapidcheck_gtest INTERFACE rapidcheck)
target_include_directories(rapidcheck_gtest INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# An INTERFACE library does not need to install anything but its headers
# and information on its targets.
install(TARGETS rapidcheck_gtest EXPORT rapidcheckConfig)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
