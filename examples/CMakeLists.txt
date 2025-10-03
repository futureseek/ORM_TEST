add_executable(quick_start_mysql quick_start_mysql.cpp)

target_include_directories(quick_start_mysql PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(quick_start_mysql PRIVATE orm_core)

# Optionally link mysql
find_package(MySQL REQUIRED)
if(MySQL_FOUND)
  target_include_directories(quick_start_mysql PRIVATE ${MySQL_INCLUDE_DIR})
  target_link_libraries(quick_start_mysql PRIVATE ${MySQL_LIBRARIES})
endif()
