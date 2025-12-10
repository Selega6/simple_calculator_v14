#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <map>
#include <string>
#include "generic_value.hpp"
#include "token_stream.hpp"
#include "token_stream.cpp"

using gv = generic_value<double>;


extern std::map<std::string, gv> names;

gv get_value(const std::string& s);
void set_value(const std::string& s, gv v);
void define_name(const std::string& s, const gv& v, bool constant=false);
bool is_declared(const std::string& s);
bool is_constant(const std::string& s);

void show_environment();
void save_env(Token filename_token);
void load_env(const std::string& filename);

#endif
