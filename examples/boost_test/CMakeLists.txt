add_executable(boost_test_integration main.cpp)
target_link_libraries(boost_test_integration rapidcheck_boost_test boost)
if (MSVC)
  target_compile_options(boost_test_integration PRIVATE "/EHa")
else()
  # Boost uses auto_ptr which causes warnings on (at least) GCC
  target_compile_options(boost_test_integration PRIVATE "-Wno-deprecated-declarations")
endif()
