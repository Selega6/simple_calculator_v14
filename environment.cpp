#include "environment.hpp"
#include "token_stream.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

using namespace std;

map<string, Value> names;

// ----------------------------
//   STRUCT VALUE
// ----------------------------

Value::Value() 
    : name{}, value{0.0}, is_const{false} {}

Value::Value(const string& n, const gv& v, bool constant)
    : name{n}, value{v}, is_const{constant} {}


// ----------------------------
//   GET / SET
// ----------------------------

gv get_value(const string& s)
{
    auto it = names.find(s);
    if (it != names.end())
        return it->second.value;

    throw runtime_error("get: undefined name " + s);
}

void set_value(const string& s, gv v)
{
    auto it = names.find(s);
    if (it == names.end())
        throw runtime_error("set: undefined name " + s);

    if (it->second.is_const)
        throw runtime_error("set: const name " + s);

    it->second.value = v;
}

bool is_declared(const string& s)
{
    return names.find(s) != names.end();
}

bool is_constant(const string& s)
{
    auto it = names.find(s);
    return it != names.end() && it->second.is_const;
}

void define_name(const string& s, const gv& v, bool constant)
{
    names[s] = Value(s, v, constant);
}


// ----------------------------
//   SHOW ENVIRONMENT
// ----------------------------

void show_environment()
{
    cout << "Environment variables:\n";

    for (const auto& p : names)
    {
        cout << "  " << p.second.name
             << " = " << p.second.value
             << (p.second.is_const ? " (const)" : "")
             << "\n";
    }
}


// ----------------------------
//   SAVE ENVIRONMENT
// ----------------------------
// Formato:
//   <nombre> <S|M> <is_const> <valor>
//   S = escalar
//   M = matriz literal completa
// ----------------------------

void save_env(Token tf)
{
    string file_name = tf.name;
    cout << "Saving environment to file: " << file_name << "\n";

    ofstream out(file_name.c_str());
    if (!out)
    {
        cerr << "Error: cannot open file " << file_name << " for writing.\n";
        return;
    }

    for (const auto& it : names)
    {
        const string& name = it.first;
        const gv& gval = it.second.value;
        bool is_const = it.second.is_const;

        // Convert actual value to string
        ostringstream oss;
        gval.to_stream(oss);
        string text = oss.str();

        // Si comienza con '{', es matriz
        if (!text.empty() && text[0] == '{')
        {
            out << name << " M " << is_const << " " << text << "\n";
        }
        else
        {
            out << name << " S " << is_const << " " << text << "\n";
        }
    }

    cout << "Environment saved to " << file_name << "\n";
}


// ----------------------------
//   LOAD ENVIRONMENT
// ----------------------------
// Lee exactamente el formato generado por save_env.
// ----------------------------

void load_env(const std::string& file_name)
{
    ifstream in(file_name.c_str());
    if (!in)
    {
        cerr << "Error: cannot open file " << file_name << " for reading.\n";
        return;
    }

    names.clear();

    string name;
    char type;
    bool is_const;

    while (in >> name >> type >> is_const)
    {
        if (type == 'S')
        {
            double x;
            in >> x;
            names[name] = Value(name, gv(x), is_const);
        }
        else if (type == 'M')
        {
            string matrix_text;
            getline(in, matrix_text);

            // limpiar espacios iniciales
            while (!matrix_text.empty() && isspace(matrix_text[0]))
                matrix_text.erase(0, 1);

            // Añadir un ";" para que el parser pueda leerlo como literal
            matrix_text += ";";

            // Crear stream temporal
            istringstream iss(matrix_text);

            // Backup del estado actual de cin
            std::streambuf* old_buf = cin.rdbuf(iss.rdbuf());

            // Backup del token_stream global
            extern Token_stream ts;
            Token_stream old_ts = ts;
            ts = Token_stream();  // reiniciar temporalmente

            // Usar el parser real para reconstruir la matriz
            extern gv statement();
            gv m = statement();

            // Restaurar estado
            ts = old_ts;
            cin.rdbuf(old_buf);

            names[name] = Value(name, m, is_const);
        }
    }

    cout << "Environment loaded from " << file_name << "\n";
}
