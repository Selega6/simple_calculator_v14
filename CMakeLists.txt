cmake_minimum_required(VERSION 2.8)

project(simple_calculator CXX)

# -----------------------------------------------------------------------------
# 1. Definición de los archivos fuente para la aplicación principal (Modular)
#    - Excluimos simple_calculator_v14.cpp porque su contenido está repartido 
#      en los demás archivos modulares (main, calculate, parser, etc.)
# -----------------------------------------------------------------------------
set(CALCULATOR_SOURCES
  main.cpp
  calculate.cpp
  parser.cpp
  environment.cpp
  token_stream.cpp
)

message(STATUS "targets:")

# -----------------------------------------------------------------------------
# 2. Programa principal: simple_calculator_v14
#    Se construye a partir de *todos* los archivos fuente modulares.
# -----------------------------------------------------------------------------
message(STATUS "  'simple_calculator_v14' will be generated (modular version)")
add_executable(
  simple_calculator_v14 
  ${CALCULATOR_SOURCES}
)

# Se enlaza la biblioteca matemática 'm' (necesaria para funciones como sin, cos, pow)
target_link_libraries(simple_calculator_v14 m)


# -----------------------------------------------------------------------------
# 3. Targets de Testing (se mantienen como estaban)
# -----------------------------------------------------------------------------
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

# Las dependencias de testing_mat_lib y testing_generic_value deberían 
# incluir los archivos de cabecera de la calculadora si los usan.
# Por ahora, se dejan como están en el original, asumiendo que sus
# dependencias están resueltas o son implícitas.