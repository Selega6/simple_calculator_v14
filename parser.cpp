#include "parser.hpp"
#include "token_stream.hpp"
#include "environment.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>

using namespace std;

extern Token_stream ts;   // definido en main o calculator
extern int precision;     // declarado en calculator

// ---------------------------------------------
//  DECLARACIONES (solo necesarias aquí)
// ---------------------------------------------
gv expression();
gv term();
gv primary();
gv function_name();


// ---------------------------------------------
//  FUNCIÓN: function_name()
// ---------------------------------------------
gv function_name()
{
    Token t = ts.get();
    if (!t.is_function())
        throw runtime_error("function name expected");

    Token tt = ts.get();
    if (!tt.is_symbol('('))
        throw runtime_error("'(' expected");

    gv v = expression();
    tt = ts.get();

    // función de un argumento
    if (tt.is_symbol(')'))
    {
        if (t.function)
            return v.call_function(t.function);
        else
            throw runtime_error(t.name + " needs two arguments");
    }

    // función de dos argumentos
    if (!tt.is_symbol(','))
        throw runtime_error("')' expected");

    gv vv = expression();
    tt = ts.get();

    if (tt.is_symbol(')'))
    {
        if (t.name == "pow")
            return v.call_function(pow, vv);
        else
            throw runtime_error(t.name + " needs only one argument");
    }

    throw runtime_error("')' expected");
}


// ---------------------------------------------
//  LIST: lista de valores separados por comas
// ---------------------------------------------
vector<typename gv::matrix_t::value_t::element_t> list()
{
    vector<typename gv::matrix_t::value_t::element_t> row;
    Token t;

    do {
        gv v = expression();
        row.push_back(v.get<typename gv::scalar_t>());
        t = ts.get();
    } 
    while (t.is_symbol(','));

    ts.unget(t);
    return row;
}


// ---------------------------------------------
//  COLUMNS: conjunto de filas matriz
// ---------------------------------------------
gv columns()
{
    vector<vector<typename gv::matrix_t::value_t::element_t>> rows;

    Token t, tt;
    do {
        t = ts.get();
        if (!t.is_symbol('{'))
            throw runtime_error("'{' expected");

        tt = ts.get();
        if (tt.is_symbol('}'))
        {
            rows.push_back({});
        }
        else
        {
            ts.unget(tt);
            rows.push_back(list());

            t = ts.get();
            if (!t.is_symbol('}'))
                throw runtime_error("'}' expected");
        }

        t = ts.get();
    }
    while (t.is_symbol(','));

    ts.unget(t);
    return gv(typename gv::matrix_t::value_t(rows));
}


// ---------------------------------------------
//  PRIMARY
// ---------------------------------------------
gv primary()
{
    Token t = ts.get();

    if (t.is_function()) 
    {
        ts.unget(t);
        return function_name();
    }

    else if (t.kind == Token::id::char_token)
    {
        if (t.is_symbol('('))
        {
            gv v = expression();
            t = ts.get();
            if (!t.is_symbol(')'))
                throw runtime_error("')' expected");
            return v;
        }
        else if (t.is_symbol('{'))
        {
            Token tt = ts.get();

            if (tt.is_symbol('{'))
            {
                ts.unget(tt);
                gv v = columns();
                t = ts.get();
                if (!t.is_symbol('}'))
                    throw runtime_error("'}' expected");
                return v;
            }
            else if (tt.is_symbol('}'))
            {
                return gv{typename gv::matrix_t::value_t()};
            }
            else
            {
                ts.unget(tt);
                gv v{typename gv::matrix_t::value_t(list())};
                t = ts.get();
                if (!t.is_symbol('}'))
                    throw runtime_error("'}' expected");
                return v;
            }
        }
        else if (t.is_symbol('-')) return -primary();
        else if (t.is_symbol('+')) return primary();
        else if (t.is_symbol('~')) return ~primary();
    }

    else if (t.kind == Token::id::number)
        return t.value;

    else if (t.kind == Token::id::name_token)
        return get_value(t.name);

    throw runtime_error("primary expected");
}


// ---------------------------------------------
//  TERM
// ---------------------------------------------
gv term()
{
    gv left = primary();

    while (true)
    {
        Token t = ts.get();

        if (t.is_symbol('*')) left = left * primary();
        else if (t.is_symbol('/')) left = left / primary();
        else if (t.is_symbol('%')) left = left % primary();
        else {
            ts.unget(t);
            return left;
        }
    }
}


// ---------------------------------------------
//  EXPRESSION
// ---------------------------------------------
gv expression()
{
    gv left = term();

    while (true)
    {
        Token t = ts.get();

        if (t.is_symbol('+')) left = left + term();
        else if (t.is_symbol('-')) left = left - term();
        else {
            ts.unget(t);
            return left;
        }
    }
}


// ---------------------------------------------
//  ASSIGN
// ---------------------------------------------
gv assign()
{
    Token t = ts.get();
    if (t.kind != Token::id::name_token)
        throw runtime_error("name expected in assign");

    string name = t.name;

    if (is_constant(name))
        throw runtime_error(name + " constant cannot be modified");

    t = ts.get();
    if (!t.is_symbol('='))
        throw runtime_error("= missing in assign of " + name);

    gv v = expression();

    if (is_declared(name)) 
        set_value(name, v);
    else
        define_name(name, v);

    return v;
}


// ---------------------------------------------
//  CONST ASSIGN
// ---------------------------------------------
gv constant_assign()
{
    Token t = ts.get();
    if (t.kind != Token::id::name_token)
        throw runtime_error("name expected in const assign");

    string name = t.name;

    if (is_declared(name))
        throw runtime_error(name + " has already been defined");

    t = ts.get();
    if (!t.is_symbol('='))
        throw runtime_error("= missing in assign of " + name);

    gv v = expression();
    define_name(name, v, true);

    return v;
}


// ---------------------------------------------
//  STATEMENT
// ---------------------------------------------
gv statement()
{
    Token t = ts.get();

    switch (t.kind)
    {
        case Token::id::const_token:
            return constant_assign();

        case Token::id::name_token:
        {
            Token tt = ts.get();

            if (tt.is_symbol('='))
            {
                ts.unget(t);
                ts.unget(tt);
                return assign();
            }
            else
            {
                ts.unget(t);
                ts.unget(tt);
                return expression();
            }
        }

        default:
            ts.unget(t);
            return expression();
    }
}
