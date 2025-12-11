#include "token_stream.hpp"
#include <iostream>
#include <cctype>
#include <sstream>
#include <cmath>
#include <stdexcept>

using namespace std;


Token::Token() 
: kind(id::none), symbol(0), value(double(0)), name(), function(nullptr)
{}

Token::Token(id tk) 
: kind(tk), symbol(0), value(double(0)), name(), function(nullptr)
{}

Token::Token(id tk, const string& str)
: kind(tk), symbol(0), value(double(0)), name(str), function(nullptr)
{}

Token::Token(char ch) 
: kind(id::char_token), symbol(ch), value(double(0)), name(), function(nullptr)
{}

Token::Token(const gv& val)
: kind(id::number), symbol(0), value(val), name(), function(nullptr)
{}

Token::Token(const string& str) 
: kind(id::name_token), symbol(0), value(double(0)), name(str), function(nullptr) 
{}

Token::Token(const string& str, double (*the_function)(double)) 
: kind(id::function_token), symbol(0), value(double(0)), name(str), function(the_function) 
{}


bool is_valid_filename(const string& s)
{
    if (s.empty()) return false;
    if (s.front() == '.' || s.back() == '.') return false;

    size_t dot_pos = s.find('.');
    if (dot_pos == string::npos) return false;
    if (dot_pos == s.length() - 1) return false;

    bool is_number = true;
    for (char c : s)
        if (!isdigit(c) && c != '.' && c != '-') { is_number = false; break; }
    if (is_number) return false;

    for (char c : s)
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-'))
            return false;

    return true;
}

Token_stream::Token_stream() { } 

Token Token_stream::get()
{
    if (!buffer.empty()) {
        // LIFO: Leer el último elemento
        Token t = buffer.back();
        buffer.pop_back();
        return t;
    }

    char ch;

    do { 
        if (!cin.get(ch)) {
            return Token(Token::id::none); 
        }
    } while (isspace(ch));

    switch (ch)
    {
        case '(': case ')':
        case '{': case '}':
        case '+': case '-':
        case '*': case '/':
        case '%': case '~':
        case '=': case ',':
            return Token(ch);

        case ';':
            return Token(Token::id::print);

        case '.': case '0': case '1': case '2':
        case '3': case '4': case '5': case '6': 
        case '7': case '8': case '9':
        {
            cin.unget();
            double val;
            if (!(cin >> val)) throw runtime_error("Error reading number");
            return Token(gv(val));
        }

        default:
            if (isalpha(ch))
            {
                string s;
                s += ch;
                while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '.' || ch == '_'))
                    s += ch;
                cin.unget();

                if (s == "quit") return Token(Token::id::quit);
                if (s == "const") return Token(Token::id::const_token);
                if (s == "help") return Token(Token::id::help_token);
                if (s == "set") return Token(Token::id::set);
                if (s == "precision") return Token(Token::id::precision_token);

                if (s == "show") return Token(Token::id::show);
                if (s == "save") return Token(Token::id::save);
                if (s == "load") return Token(Token::id::load);
                if (s == "env") return Token(Token::id::env);

                if (s == "sin") return Token(s, ::sin);
                if (s == "cos") return Token(s, ::cos);
                if (s == "tan") return Token(s, ::tan);
                if (s == "asin") return Token(s, ::asin);
                if (s == "acos") return Token(s, ::acos);
                if (s == "atan") return Token(s, ::atan);
                if (s == "exp") return Token(s, ::exp);

                if (s == "pow") return Token(s, nullptr);
                if (s == "ln") return Token(s, ::log);
                if (s == "log10") return Token(s, ::log10);
                if (s == "log2") return Token(s, ::log2);

                if (is_valid_filename(s))
                    return Token(Token::id::filename, s);

                return Token(s);
            }

            throw runtime_error("Bad token: " + string(1, ch));
    }
}



void Token_stream::ignore()
{
    while (!buffer.empty()) {
        Token t = buffer.back(); 
        buffer.pop_back();      
        if (t.kind == Token::id::quit) return;
    }

    char ch;
    while (cin >> ch)
        if (ch == ';') return;
}

void Token_stream::unget(Token t)
{ 
    buffer.push_back(t); 
}