cmake_minimum_required(VERSION 2.8)

project(mat_lib CXX)

#message(STATUS "libraries:")
#
#message(STATUS "  'mat_lib' will be generated")
## mat_lib library 
#add_library(
#  mat_lib
#  matrix.cpp 
#)

message(STATUS "targets:")
# testing_mat_lib 
message(STATUS "  'testing_mat_lib' will be generated")
add_executable(
  testing_mat_lib 
  testing_mat_lib.cpp
)
#target_link_libraries(testing_mat_lib mat_lib)

message(STATUS "  'testing_generic_value' will be generated")
add_executable(
  testing_generic_value 
  testing_generic_value.cpp
)

message(STATUS "  'simple_calculator_v14' will be generated")
add_executable(
  simple_calculator_v14 
  simple_calculator_v14.cpp
)

# message(STATUS "  'parser' will be generated")
# add_executable(
#   parser 
#   parser.cpp
# )

# message(STATUS "  'token_stream' will be generated")
# add_executable(
#   token_stream 
#   token_stream.cpp
# )

# message(STATUS "  'environment' will be generated")
# add_executable(
#   environment 
#   environment.cpp
# )

# message(STATUS "  'calculate' will be generated")
# add_executable(
#   calculate 
#   calculate.cpp
# )
# message(STATUS "  'main' will be generated")
# add_executable(
#   main 
#   main.cpp
# )
