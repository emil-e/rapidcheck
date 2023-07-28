add_library(rapidcheck_doctest INTERFACE)
target_link_libraries(rapidcheck_doctest INTERFACE rapidcheck)
target_include_directories(rapidcheck_doctest INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# An INTERFACE library does not need to install anything but its headers
# and information on its targets.
install(TARGETS rapidcheck_doctest EXPORT rapidcheckConfig)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
