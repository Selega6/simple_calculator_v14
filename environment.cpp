#include "environment.hpp"
#include "token_stream.hpp"
#include "generic_function.hpp"
#include "generic_value.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <sstream>
#include <cctype>

bool DEBUG_MODE_ENV = false;
using namespace std;
map<string, VariableEntry> names;
using gv=generic_value<double>;

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
    names[s] = VariableEntry(s, v, constant);
}



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
    for (const auto& f : generic_functions)
    {
        cout << "Function: " << f.second.name << "(";
        for (size_t i = 0; i < f.second.params.size(); ++i)
        {
            cout << f.second.params[i];
            if (i < f.second.params.size() - 1)
                cout << ", ";
        }
        cout << ") = " << f.second.body_expression << "\n";
    }
}


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
    const std::string& name = it.first;
    gv gval = it.second.value;
    bool is_const = it.second.is_const;

    try
    {
        gval.get<gv::matrix_t>();

        std::ostringstream oss;
        gval.to_stream(oss);

        std::string text = cleanup_matrix_output(oss.str());
        out << name << " M " << is_const << " " << text << "\n";
    }
    catch (const std::logic_error&)
    {
        std::ostringstream oss;
        gval.to_stream(oss);

        out << name << " S " << is_const << " " << oss.str() << "\n";
    }
}




    for (const auto& it : generic_functions)
    {
        const GenericFunction& f = it.second;

        out << f.name << " F " << 0 << " " << f.params.size();

        for (const auto& p : f.params)
            out << " " << p;

        out << " \"" << f.body_expression << "\"\n";
    }

    cout << "Environment saved to " << file_name << "\n";
}


void load_env(const std::string& file_name)
{
    ifstream in(file_name.c_str());
    if (!in)
    {
        cerr << "Error: cannot open file " << file_name << " for reading.\n";
        return;
    }

    names.clear();
    generic_functions.clear();

    string name;
    char type;
    bool is_const;

    while (in >> name >> type >> is_const)
    {
        if (DEBUG_MODE_ENV)
        {
            cout << "Loading entry: " << name << " Type: " << type << "\n";
        }
        if (type == 'S')
        {
            double x;
            in >> x;
            names[name] = VariableEntry(name, gv(x), is_const);
        }
        else if (type == 'M')
        {
            string matrix_text;
            getline(in, matrix_text);

            while (!matrix_text.empty() && isspace(matrix_text[0]))
                matrix_text.erase(0, 1);

            matrix_text += ";";

            istringstream iss(matrix_text);

            std::streambuf* old_buf = cin.rdbuf(iss.rdbuf());

            extern Token_stream ts;
            Token_stream old_ts = ts;
            ts = Token_stream();

            extern gv statement();
            gv m = statement();

            ts = old_ts;
            cin.rdbuf(old_buf);

            names[name] = VariableEntry(name, m, is_const);
        }
        else if (type == 'F')
        {
            size_t param_count;
            in >> param_count;

            vector<string> params(param_count);
            for (size_t i = 0; i < param_count; ++i)
                in >> params[i];

            string line;
            getline(in, line);

            while (!line.empty() && isspace(line[0]))
                line.erase(0, 1);

            if (line.size() >= 2 && line.front() == '"' && line.back() == '"')
                line = line.substr(1, line.size() - 2);

            generic_functions[name] = GenericFunction(name, params, line);

            if (DEBUG_MODE_ENV)
            {
                cerr << "Loaded function " << name << "(";
                for (size_t i = 0; i < params.size(); ++i)
                {
                    cerr << params[i];
                    if (i + 1 < params.size()) cerr << ", ";
                }
                cerr << ") = " << line << endl;
            }
}


        else
        {
            cerr << "Error: unknown entry type '" << type << "' in environment file.\n";
        }
    }

    cout << "Environment loaded from " << file_name << "\n";
}
