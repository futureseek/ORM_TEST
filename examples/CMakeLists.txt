add_executable(quick_start_mysql quick_start_mysql.cpp)

target_include_directories(quick_start_mysql PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(quick_start_mysql PRIVATE orm_core)

# Optionally link mysql
find_library(MYSQLCPPCONN_LIB NAMES mysqlcppconn)
find_path(MYSQLCPPCONN_INCLUDE_DIR NAMES cppconn/driver.h)
if(MYSQLCPPCONN_LIB AND MYSQLCPPCONN_INCLUDE_DIR)
  target_include_directories(quick_start_mysql PRIVATE ${MYSQLCPPCONN_INCLUDE_DIR})
  target_link_libraries(quick_start_mysql PRIVATE ${MYSQLCPPCONN_LIB})
  message(STATUS "Found mysqlcppconn for example: ${MYSQLCPPCONN_LIB}")
else()
  message(FATAL_ERROR "mysqlcppconn (Connector/C++) not found. Install libmysqlcppconn and rerun CMake")
endif()
