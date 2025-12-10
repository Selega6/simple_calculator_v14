#ifndef PARSER_HPP
#define PARSER_HPP

#include "generic_value.hpp"

using gv = generic_value<double>;

gv expression();
gv term();
gv primary();
gv statement();
gv assign();
gv constant_assign();

#endif
