#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <map>
#include <string>
#include "generic_value.hpp"
#include "token_stream.hpp" 

using gv = generic_value<double>;

struct VariableEntry {
    std::string name;
    gv value;
    bool is_const; 

    VariableEntry() : name{}, value{0.0}, is_const{false} {} 
    VariableEntry(const std::string& n, const gv& v, bool constant)
        : name{n}, value{v}, is_const{constant} {}
};


extern std::map<std::string, VariableEntry> names; 

gv get_value(const std::string& s);
void set_value(const std::string& s, gv v);
void define_name(const std::string& s, const gv& v, bool constant=false);
bool is_declared(const std::string& s);
bool is_constant(const std::string& s);

void show_environment();
void save_env(Token filename_token);
void load_env(const std::string& filename);

#endif