add_executable(database main.cpp
  Database.cpp
  DatabaseConnection.cpp)

target_link_libraries(database rapidcheck)