add_library(orm_core
  # ... implementation sources will go here
)

target_include_directories(orm_core PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

# Link mysqlcppconn if found (optional)
find_package(MySQL REQUIRED)
if(MySQL_FOUND)
  target_include_directories(orm_core PRIVATE ${MySQL_INCLUDE_DIR})
  target_link_libraries(orm_core PRIVATE ${MySQL_LIBRARIES})
endif()
