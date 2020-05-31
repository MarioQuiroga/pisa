add_executable(queries_with_caching queries_with_caching.cpp)
target_link_libraries(queries_with_caching
  pisa
  CLI11
)
add_executable(queries queries.cpp)
target_link_libraries(queries
  pisa
  CLI11
)