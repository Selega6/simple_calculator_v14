#include "calculate.hpp"
#include "parser.hpp"
#include "token_stream.hpp"
#include "environment.hpp"
#include "generic_function.hpp"

#include <iostream>
#include <iomanip>
#include <stdexcept>

using namespace std;

extern Token_stream ts;
extern int precision;

std::map<std::string, GenericFunction> generic_functions;
std::map<std::string, std::string> generic_function_signatures;


void clean_up_mess()
{
    ts.ignore();
}


void help()
{
    cout
    << " simple_calculator - matrix + scalar interpreter\n"
    << "\n"
    << " This is a simple calculator supporting arithmetic,\n"
    << " variables, constants, and matrices.\n"
    << "\n"
    << " Supported functions:\n"
    << "   sin, cos, tan, asin, acos, atan, exp, pow, ln, log10, log2\n"
    << "\n"
    << " Examples:\n"
    << "   2 + 3*7 - (8 - 3.2) - 1/3;\n"
    << "   a = 3;\n"
    << "   const pi = 3.141592;\n"
    << "   sin(2*pi/4);\n"
    << "   pow(3,2);\n"
    << "\n"
    << " Matrices:\n"
    << "   {{1,2},{3,4}};\n"
    << "   {1,2,3,4};   (row vector)\n"
    << "\n"
    << " Commands:\n"
    << "   help               → show this help\n"
    << "   quit               → exit calculator\n"
    << "   precision;         → show printed precision\n"
    << "   set precision N;   → set precision\n"
    << "   show env;          → list variables\n"
    << "   save env <file>;   → save environment\n"
    << "   load env <file>;   → load environment\n"
    << "\n"
    << " End each expression with a semicolon.\n"
    << endl;
}


void precision_statement()
{
    cout << " precision digits: " << precision << "\n";
}

void set_precision()
{
    Token t = ts.get();
    if (t.kind != Token::id::precision_token)
        throw runtime_error("precision keyword expected");

    gv d = expression();  
    precision = d.get<gv::scalar_t>();

    cout << " precision set to " << precision << " digits\n";
}


void calculate()
{
    while (true)
    {
        try 
        {
            cout << "> ";

            Token t = ts.get();
            while (t.kind == Token::id::print)
                t = ts.get();

            if (t.kind == Token::id::quit) 
                return;

            if (t.kind == Token::id::help_token)
            {
                help();
                continue;
            }

            if (t.kind == Token::id::precision_token)
            {
                precision_statement();
                continue;
            }

            if (t.kind == Token::id::set)
            {
                set_precision();
                continue;
            }

            if (t.kind == Token::id::show)
            {
                Token tt = ts.get();
                if (tt.kind == Token::id::env)
                    show_environment();
                else
                {
                    ts.unget(tt);
                    throw runtime_error("env should follow 'show'");
                }
                continue;
            }

            if (t.kind == Token::id::save)
            {
                Token tt = ts.get();
                if (tt.kind == Token::id::env)
                {
                    Token tf = ts.get();
                    if (tf.kind == Token::id::filename)
                        save_env(tf);
                    else 
                    {
                        ts.unget(tf);
                        throw runtime_error("filename should follow 'save env'");
                    }
                }
                else
                {
                    ts.unget(tt);
                    throw runtime_error("env should follow 'save'");
                }
                continue;
            }

            if (t.kind == Token::id::load)
            {
                Token tt = ts.get();
                if (tt.kind == Token::id::env)
                {
                    Token tf = ts.get();
                    if (tf.kind == Token::id::filename)
                        load_env(tf.name);
                    else
                    {
                        ts.unget(tf);
                        throw runtime_error("filename should follow 'load env'");
                    }
                }
                else
                {
                    ts.unget(tt);
                    throw runtime_error("env should follow 'load'");
                }
                continue;
            }

            ts.unget(t);
            gv r = statement();

            cout << fixed << setprecision(precision)
                 << "= " << r << "\n";
        }
        catch (exception& e)
        {
            cerr << e.what() << "\n";
            clean_up_mess();
        }
    }
}
