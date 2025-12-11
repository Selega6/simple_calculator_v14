cmake_minimum_required(VERSION 2.8)

project(simple_calculator CXX)

set(CALCULATOR_SOURCES
  main.cpp
  calculate.cpp
  parser.cpp
  environment.cpp
  token_stream.cpp
)

message(STATUS "targets:")

message(STATUS "  'simple_calculator_v14' will be generated (modular version)")
add_executable(
  simple_calculator_v14 
  ${CALCULATOR_SOURCES}
)

target_link_libraries(simple_calculator_v14 m)


message(STATUS "  'testing_mat_lib' will be generated")
add_executable(
  testing_mat_lib 
  testing_mat_lib.cpp
)

message(STATUS "  'testing_generic_value' will be generated")
add_executable(
  testing_generic_value 
  testing_generic_value.cpp
)
