project(rapidcheck_boost CXX)

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

set(PKG_CONFIG_REQUIRES "rapidcheck")
set(PKG_CONFIG_DESCRIPTION_SUMMARY "boost headers for rapidcheck")
set(PKG_CONFIG_VERSION)
set(PKG_CONFIG_LIBDIR "\${prefix}/lib")
set(PKG_CONFIG_INCLUDEDIR "\${prefix}/include")
set(PKG_CONFIG_LIBS)
set(PKG_CONFIG_CFLAGS "-I\${includedir}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/../../pkg-config.pc.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
  )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  )
