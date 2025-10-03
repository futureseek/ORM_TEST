cmake_minimum_required(VERSION 3.10)
project(orm_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ORM_BUILD_EXAMPLES "Build examples" ON)
option(ORM_BUILD_TESTS "Build tests" ON)

add_subdirectory(src)

if(ORM_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(ORM_BUILD_TESTS)
  add_subdirectory(tests)
endif()
