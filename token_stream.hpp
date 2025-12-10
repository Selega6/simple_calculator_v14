#ifndef TOKEN_STREAM_HPP
#define TOKEN_STREAM_HPP

#include <queue>
#include <string>
#include "generic_value.hpp"

using gv = generic_value<double>;

struct Token {
    enum id {
        none, quit, print, number, name_token, const_token, char_token,
        help_token, function_token, precision_token, set, show, save,
        load, env, filename
    };

    id kind;
    char symbol;
    gv value;
    std::string name;
    double (*function)(double);

    Token();
    Token(id);
    Token(char);
    Token(const gv&);
    Token(const std::string&);
    Token(const std::string&, double(*func)(double));

    bool is_symbol(char c);
    bool is_function();
};

class Token_stream {
public:
    Token_stream();
    Token get();
    void unget(Token t);
    void ignore();

private:
    std::queue<Token> buffer;
};

// función auxiliar
bool is_valid_filename(const std::string& s);

#endif
