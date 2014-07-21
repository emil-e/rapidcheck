add_executable(counter main.cpp)

target_link_libraries(counter rapidcheck)
target_include_directories(counter PRIVATE ../../ext/catch/include)