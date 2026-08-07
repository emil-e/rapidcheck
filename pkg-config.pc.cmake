prefix=${CMAKE_INSTALL_PREFIX}
includedir=${CMAKE_INSTALL_FULL_INCLUDEDIR}
libdir=${CMAKE_INSTALL_FULL_LIBDIR}

Name: ${PROJECT_NAME}
Description: ${PKG_CONFIG_DESCRIPTION_SUMMARY}
Version: 
Libs: -L${libdir} -lrapidcheck
Cflags: -I${includedir}
