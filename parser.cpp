#include "parser.hpp"
#include "token_stream.hpp"
#include "environment.hpp"
#include "generic_function.hpp"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <cmath>
#include <locale>

using namespace std;
extern bool DEBUG_MODE;
bool DEBUG_MODE = true;
extern Token_stream ts;
extern int precision;
extern std::map<std::string, GenericFunction> generic_functions;

gv expression();
gv term();
gv primary();
gv function_name();

static std::string cleanup_matrix_output(const std::string& text)
{
    std::string cleaned_text;
    cleaned_text.reserve(text.size());

    for (char c : text)
    {
        if (!std::isspace(static_cast<unsigned char>(c)))
        {
            cleaned_text += c;
        }
    }
    return cleaned_text;
}

static std::string replace_parameters(
    const GenericFunction& f,
    const std::vector<gv>& args)
{
    if (DEBUG_MODE)
        std::cerr << "DEBUG replace_parameters: Function '" << f.name
                  << "' arity=" << f.params.size() << " args=" << args.size() << "\n";

    if (args.size() != f.params.size())
        throw runtime_error("wrong number of arguments for function " + f.name);

    std::string expr = f.body_expression;

    if (DEBUG_MODE)
        std::cerr << "DEBUG replace_parameters: Original body: '" << expr << "'\n";

    for (size_t i = 0; i < f.params.size(); ++i)
    {
        std::ostringstream ss;
        args[i].to_stream(ss);

        std::string arg_value_str = ss.str();

        if (!arg_value_str.empty() && arg_value_str[0] == '\n')
        {
            arg_value_str.erase(0, 1); 
        }


        if (DEBUG_MODE)
            std::cerr << "DEBUG replace_parameters: Replace '" << f.params[i]
                      << "' -> '" << arg_value_str << "'\n";

        size_t pos = 0;
        while ((pos = expr.find(f.params[i], pos)) != std::string::npos)
        {
            expr.replace(pos, f.params[i].size(), arg_value_str);
            pos += arg_value_str.size();
        }
    }

    if (DEBUG_MODE)
        std::cerr << "DEBUG replace_parameters: Final expression: '" << expr << "'\n";

    return expr;
}

gv call_generic_function(
    const GenericFunction& f,
    const std::vector<gv>& args)
{
    if (DEBUG_MODE)
        std::cerr << "DEBUG call_generic_function: START '" << f.name << "'\n";

    std::string processed = replace_parameters(f, args);

    std::istringstream iss(processed);
    iss.imbue(std::locale::classic());

    if (DEBUG_MODE)
        std::cerr << "DEBUG call_generic_function: parser on '" << processed << "'\n";

    std::streambuf* oldbuf = std::cin.rdbuf(iss.rdbuf());

    extern Token_stream ts;
    Token_stream ts_backup = ts;
    ts = Token_stream();

    gv result;

    try
    {
        result = expression();

        if (DEBUG_MODE)
            std::cerr << "DEBUG call_generic_function: parser result ok\n";
    }
    catch (...)
    {
        std::cin.rdbuf(oldbuf);
        ts = ts_backup;

        if (DEBUG_MODE)
            std::cerr << "DEBUG call_generic_function: ERROR, state restored\n";

        throw;
    }

    std::cin.rdbuf(oldbuf);
    ts = ts_backup;

    if (DEBUG_MODE)
        std::cerr << "DEBUG call_generic_function: END\n";

    return result;
}

gv function_name()
{
    Token t = ts.get();
    if (!t.is_function())
        throw runtime_error("function name expected");

    Token tt = ts.get();
    if (!tt.is_symbol('('))
        throw runtime_error("'(' expected");

    gv v = primary();

    tt = ts.get();
    if (tt.is_symbol(')'))
    {
        if (t.name == "inv")
        {
            auto M = v.get<gv::matrix_t>();
            return gv(M.inverse());
        }

        if (t.function)
            return v.call_function(t.function);

        throw runtime_error(t.name + " needs one argument");
    }

    if (!tt.is_symbol(','))
        throw runtime_error("')' expected or ','");

    gv vv = expression();
    tt = ts.get();

    if (!tt.is_symbol(')'))
        throw runtime_error("')' expected");

    if (t.name == "pow")
        return v.call_function(pow, vv);

    throw runtime_error(t.name + " needs two arguments");
}

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

gv columns()
{
    vector<vector<typename gv::matrix_t::value_t::element_t>> rows_data;

    Token t = ts.get();
    if (!t.is_symbol('{'))
        throw runtime_error("'{' expected to start matrix/vector literal");

    Token next = ts.get();
    ts.unget(next);
    
    if (next.is_symbol('{'))
    {
        Token t_row_start, t_row_end, t_sep;
        do {
            t_row_start = ts.get();
            if (!t_row_start.is_symbol('{'))
            {
                ts.unget(t_row_start);
                break; 
            }
            
            Token tt = ts.get();
            if (tt.is_symbol('}'))
            {
                rows_data.push_back({});
            }
            else
            {
                ts.unget(tt);
                rows_data.push_back(list());
                
                t_row_end = ts.get();
                if (!t_row_end.is_symbol('}'))
                    throw runtime_error("'}' expected for matrix row end");
            }

            t_sep = ts.get();
        }
        while (t_sep.is_symbol(','));

        if (!t_sep.is_symbol('}'))
        {
            ts.unget(t_sep); 
            throw runtime_error("'}' expected at the end of matrix literal");
        }
    }
    else
    {

        if (next.is_symbol('}'))
        {
             ts.get();
        }
        else
        {
            vector<typename gv::matrix_t::value_t::element_t> row_elements = list();

            Token t_close = ts.get();
            if (!t_close.is_symbol('}'))
            {
                ts.unget(t_close);
                throw runtime_error("'}' expected after matrix row vector");
            }
            
            rows_data.push_back(row_elements);
        }
    }

    if (rows_data.size() > 1)
    {
        size_t cols = 0;
        bool first = true;
        for (const auto& row : rows_data)
        {
            if (row.empty()) continue;

            if (first)
            {
                cols = row.size();
                first = false;
            }
            else if (row.size() != cols)
            {
                throw runtime_error("column length mismatch in matrix literal");
            }
        }
    }


    return gv(typename gv::matrix_t::value_t(rows_data));
}

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

gv primary()
{
    Token t = ts.get();

    if (DEBUG_MODE)
        std::cerr << "DEBUG primary: token name=" << t.name << "\n";

    if (t.is_symbol('+'))
    {
        return primary(); 
    }
    
    if (t.is_symbol('-'))
    {
        return -primary();
    }

    if (t.is_function())
    {
        if (DEBUG_MODE)
            std::cerr << "DEBUG primary: built-in function " << t.name << "\n";
            std::cerr << "DEBUG primary: parsing built-in function\n";

        ts.unget(t);
        return function_name();
    }

    if (t.kind == Token::id::name_token)
    {
        string fname = t.name;

        if (DEBUG_MODE)
            std::cerr << "DEBUG primary: name token '" << fname << "'\n";

        Token t2 = ts.get();

        if (t2.is_symbol('('))
        {
            if (DEBUG_MODE)
                std::cerr << "DEBUG primary: parsing call to " << fname << "\n";

            vector<gv> args;

            Token arg = ts.get();
            if (!arg.is_symbol(')'))
            {
                ts.unget(arg);

                while (true)
                {
                    args.push_back(expression());

                    if (DEBUG_MODE)
                        std::cerr << "DEBUG primary arg: " << args.back() << "\n";

                    Token sep = ts.get();
                    if (sep.is_symbol(')')) break;
                    if (!sep.is_symbol(','))
                        throw runtime_error("',' or ')' expected");
                }
            }

            if (generic_functions.count(fname) == 0)
                throw runtime_error("undefined function: " + fname);

            if (DEBUG_MODE)
                std::cerr << "DEBUG primary: calling generic function\n";

            return call_generic_function(generic_functions[fname], args);
        }

        ts.unget(t2);

        if (DEBUG_MODE)
            std::cerr << "DEBUG primary: returning variable '" << fname << "'\n";

        return get_value(fname);
    }

    if (t.kind == Token::id::number)
        return t.value;

    if (t.is_symbol('{'))
    {
        ts.unget(t);
        return columns();
    }
    
    if (t.is_symbol('('))
    {
        gv v = expression();
        t = ts.get();

        if (!t.is_symbol(')'))
            throw runtime_error("')' expected");

        return v;
    }

    if (DEBUG_MODE)
        std::cerr << "DEBUG primary: ERROR token\n";

    throw runtime_error("primary expected");
}

static std::string read_function_body()
{
    std::ostringstream body;

    if (DEBUG_MODE)
        std::cerr << "DEBUG read_function_body: start\n";

    while (true)
    {
        Token tk = ts.get();

        if (tk.kind == Token::id::print)
        {
            if (DEBUG_MODE)
                std::cerr << "DEBUG read_function_body: ';' found\n";
            break;
        }

        if (tk.kind == Token::id::quit)
            throw runtime_error("missing ';' after function body");

        if (tk.kind == Token::id::number)
        {
            std::ostringstream ss;
            tk.value.to_stream(ss);
            body << ss.str();
        }
        else if (
            tk.kind == Token::id::name_token ||
            tk.kind == Token::id::function_token 
        )
        {
            body << tk.name;
        }
        else if (tk.kind == Token::id::char_token)
        {
            body << tk.symbol;
        }
        else
        {
            if (DEBUG_MODE)
                std::cerr << "DEBUG read_function_body: BAD TOKEN\n";
            throw runtime_error("bad token in function body");
        }
    }

    if (DEBUG_MODE)
        std::cerr << "DEBUG read_function_body: final body '" << body.str() << "'\n";

    return body.str();
}


gv statement()
{
    Token t = ts.get();
    string fname;

    if (t.kind == Token::id::const_token)
    {
        if (DEBUG_MODE)
            std::cerr << "DEBUG statement: CONST\n";
        return constant_assign();
    }

    if (t.kind == Token::id::name_token)
    {
        fname = t.name;
        Token t_open = ts.get();

        if (t_open.is_symbol('('))
        {
            if (DEBUG_MODE)
                std::cerr << "DEBUG statement: potential function definition\n";

            std::vector<Token> consumed;
            consumed.push_back(t_open);

            Token p = ts.get();
            consumed.push_back(p);

            std::vector<std::string> params;
            bool is_def = true;

            if (p.kind == Token::id::name_token)
            {
                params.push_back(p.name);
                Token sep = ts.get();
                consumed.push_back(sep);

                while (sep.is_symbol(','))
                {
                    Token pn = ts.get();
                    consumed.push_back(pn);

                    if (pn.kind != Token::id::name_token)
                        is_def = false;

                    params.push_back(pn.name);

                    sep = ts.get();
                    consumed.push_back(sep);
                }

                if (!sep.is_symbol(')'))
                    is_def = false;
            }
            else if (!p.is_symbol(')'))
            {
                is_def = false;
            }

            Token eq = ts.get();

            if (is_def && eq.is_symbol('='))
            {
                if (DEBUG_MODE)
                    std::cerr << "DEBUG statement: confirmed function definition\n";

                std::string body_expr = read_function_body();
                generic_functions[fname] =
                    GenericFunction(fname, params, body_expr);

                std::cout << "Function " << fname << "(";
                for (size_t i = 0; i < params.size(); ++i)
                {
                    std::cout << params[i]
                              << (i < params.size() - 1 ? "," : "");
                }
                std::cout << ") defined.\n";

                return gv(0.0);
            }
            else
            {
                if (DEBUG_MODE)
                    std::cerr << "DEBUG statement: rewinding (not a def)\n";

                ts.unget(eq);

                for (auto it = consumed.rbegin(); it != consumed.rend(); ++it)
                    ts.unget(*it);

                ts.unget(t);
            }
        }
        else
        {
            ts.unget(t_open);
            ts.unget(t);
        }
    }
    
    if (t.kind != Token::id::name_token)
    {
        ts.unget(t);
        return expression();
    }

    Token tt = ts.get();

    if (tt.kind == Token::id::name_token)
    {
        Token eq = ts.get();
        if (eq.is_symbol('='))
        {
            ts.unget(eq);
            ts.unget(tt);
            return assign();
        }

        ts.unget(eq);
        ts.unget(tt);
        return expression();
    }

    ts.unget(tt);
    return expression();
}