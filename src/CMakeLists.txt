add_library(orm_core STATIC
  connection_pool.cpp
  mysql_driver.cpp
  session.cpp
  transaction.cpp
  # future implementation sources:  query_builder.cpp
)

target_include_directories(orm_core PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

# Link mysqlcppconn if found (optional)
find_library(MYSQLCPPCONN_LIB NAMES mysqlcppconn)
find_path(MYSQLCPPCONN_INCLUDE_DIR NAMES cppconn/driver.h)
if(MYSQLCPPCONN_LIB AND MYSQLCPPCONN_INCLUDE_DIR)
  target_include_directories(orm_core PRIVATE ${MYSQLCPPCONN_INCLUDE_DIR})
  target_link_libraries(orm_core PRIVATE ${MYSQLCPPCONN_LIB})
  message(STATUS "Found mysqlcppconn: ${MYSQLCPPCONN_LIB}")
else()
  message(FATAL_ERROR "mysqlcppconn (Connector/C++) not found. Install libmysqlcppconn and rerun CMake")
endif()
