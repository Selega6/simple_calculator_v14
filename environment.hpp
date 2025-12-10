#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <map>
#include <string>
#include "generic_value.hpp"
#include "token_stream.hpp" 
// Se ha eliminado: #include "token_stream.cpp"

using gv = generic_value<double>;

// 1. DEFINICIÓN DEL STRUCT VALUE (AÑADIDO)
struct Value 
{
  std::string name;
  gv value;
  bool is_const;

  Value();
  Value(const std::string& n, const gv& v, bool constant = false); 
};


// 2. DECLARACIÓN EXTERNA DE LA TABLA DE NOMBRES (CORREGIDA)
// Ahora usa el tipo correcto 'Value'
extern std::map<std::string, Value> names; 

// Declaraciones de funciones
gv get_value(const std::string& s);
void set_value(const std::string& s, gv v);
void define_name(const std::string& s, const gv& v, bool constant=false);
bool is_declared(const std::string& s);
bool is_constant(const std::string& s);

void show_environment();
void save_env(Token filename_token);
void load_env(const std::string& filename);

#endif