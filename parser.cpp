#include "parser.hpp"
#include "token_stream.hpp"
#include "environment.hpp"
#include "generic_function.hpp" // Necesario para GenericFunction

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <cmath>
#include <locale> // Asegúrate de incluir esta librería
using namespace std;

// Declaraciones externas (necesarias para el enlazador)
extern Token_stream ts;
extern int precision;
extern std::map<std::string, GenericFunction> generic_functions;

// Declaraciones internas para funciones del parser
gv expression();
gv term();
gv primary();
gv function_name();

// ---------------------------------------------
//  Funciones Auxiliares para Funciones Genéricas
//  (Traídas de simple_calculator_v14.cpp e implementadas aquí)
// ---------------------------------------------

/**
 * Reemplaza los parámetros formales en el cuerpo de la función con los valores
 * reales de los argumentos.
 */
/**
 * Reemplaza los parámetros formales en el cuerpo de la función con los valores
 * reales de los argumentos.
 */
/**
 * Reemplaza los parámetros formales en el cuerpo de la función con los valores
 * reales de los argumentos.
 */
static std::string replace_parameters(
    const GenericFunction& f,
    const std::vector<gv>& args)
{
    if(args.size() != f.params.size())
        throw runtime_error("wrong number of arguments for function " + f.name);

    std::string expr = f.body_expression;

    for(size_t i = 0; i < f.params.size(); ++i)
    {
        std::ostringstream ss;
        // Asumiendo que gv::to_stream es el método para convertir a string
        args[i].to_stream(ss); 

        size_t pos = 0;
        while((pos = expr.find(f.params[i], pos)) != std::string::npos)
        {
            expr.replace(pos, f.params[i].size(), ss.str());
            pos += ss.str().size();
        }
    }
    return expr;
}
/**
 * Llama a la función genérica, ejecutando su cuerpo en un contexto temporal.
 */
// Archivo: parser.cpp (Corrección en call_generic_function)

// ... (todo el código anterior) ...

/**
 * Llama a la función genérica, ejecutando su cuerpo en un contexto temporal.
 */
/**
 * Llama a la función genérica, ejecutando su cuerpo en un contexto temporal.
 */
/**
 * Llama a la función genérica, ejecutando su cuerpo en un contexto temporal.
 */
gv call_generic_function(
    const GenericFunction& f,
    const std::vector<gv>& args)
{
    // 1. Reemplazar parámetros y preparar la expresión para la evaluación.
    std::string processed = replace_parameters(f, args);
    
    // --- CORRECCIÓN APLICADA AQUÍ ---
    // NO añadimos el punto y coma (;) al final de la expresión procesada.
    // Esto previene que el parser interno consuma el token de fin de sentencia
    // que el parser principal espera al final de la línea.
    // processed += ";"; // LÍNEA ELIMINADA O COMENTADA
    // --------------------------------

    std::istringstream iss(processed);
    
    // CORRECCIÓN CLAVE: Forzar el locale clásico (C) para asegurar el punto decimal.
    iss.imbue(std::locale::classic()); 

    // 2. Guardar y Redirigir la Entrada Estándar (std::cin)
    std::streambuf* oldbuf = std::cin.rdbuf(iss.rdbuf());

    // 3. Guardar y Reiniciar el Token_stream Global
    extern Token_stream ts;
    Token_stream ts_backup = ts;
    ts = Token_stream(); 

    gv result;
    try {
        // Ejecutar el parser recursivamente
        result = expression(); 
    } catch (...) {
        // 4. Restaurar el estado en caso de error
        std::cin.rdbuf(oldbuf);
        ts = ts_backup;
        throw;
    }

    // 5. Restaurar cin y el Token_stream global
    std::cin.rdbuf(oldbuf);
    ts = ts_backup;

    return result;
}

// ... (todo el código posterior) ...


// ---------------------------------------------
//  FUNCIÓN: function_name() (para funciones internas como sin/cos)
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
        throw runtime_error("')' expected or ',' for two arguments");

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
//  PRIMARY (CORREGIDA para manejar funciones genéricas)
// ---------------------------------------------
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
//  STATEMENT (Se mantiene la lógica de definición de función ya corregida)
// ---------------------------------------------
// Archivo: parser.cpp (Solo la función statement() corregida)

// Archivo: parser.cpp (Función statement() y primary() estabilizadas)

// ... (El resto del código de includes, declaraciones y funciones auxiliares) ...

// ---------------------------------------------
//  PRIMARY (CORREGIDA para manejar llamadas a función)
// ---------------------------------------------
gv primary()
{
    Token t = ts.get();

    // 1. Funciones internas (sin, cos, etc.)
    if (t.is_function()) 
    {
        ts.unget(t);
        return function_name();
    }

    // 2. Nombre: Variable o Llamada a Función Genérica
    if (t.kind == Token::id::name_token)
    {
        string fname = t.name;
        Token t2 = ts.get(); // Peek ahead for '('

        if (t2.is_symbol('(')) // ES UNA LLAMADA A FUNCIÓN GENÉRICA: f(1,2)
        {
            vector<gv> args;
            
            // Revisa si es una llamada vacía f()
            Token arg_token = ts.get();
            if (!arg_token.is_symbol(')'))
            {
                // No es vacía, devolvemos el token y empezamos a leer argumentos.
                ts.unget(arg_token);
                
                // Lee la lista de expresiones separadas por comas.
                while(true)
                {
                    args.push_back(expression());
                    Token sep = ts.get();
                    
                    if (sep.is_symbol(')')) break; // Fin de la lista de argumentos
                    
                    if (!sep.is_symbol(',')) 
                        throw runtime_error("',' or ')' expected in function call");
                    
                    // Si es ',', continuamos el ciclo.
                }
            }
            // El token ')' (o el token después del último argumento) ha sido consumido.
            
            if (generic_functions.count(fname) == 0)
                throw runtime_error("undefined function: " + fname);

            return call_generic_function(generic_functions[fname], args);
        }

        // Si no es '(', es una variable. Devolvemos el token peeked.
        ts.unget(t2); 

        // 3. Es una Variable o Constante
        return get_value(fname); 
    }
    
    // ... (El resto de Primary, igual) ...
    
    if (t.kind == Token::id::number)
        return t.value;

    throw runtime_error("primary expected");
}


// ---------------------------------------------
//  STATEMENT (Flujo de control estabilizado)
// ---------------------------------------------
gv statement()
{
    Token t = ts.get();

    // 1. Lógica de definición de función: f(x,y)=x+y;
    if(t.kind == Token::id::name_token)
    {
        string fname = t.name;
        Token t2 = ts.get(); // Peek: '('

        // Si aparece '(' tras un nombre → Posible definición o llamada.
        if(t2.is_symbol('('))
        {
            // Creamos un vector temporal para guardar los tokens de los parámetros 
            // leídos durante el chequeo de la definición.
            std::vector<Token> consumed_tokens;
            
            // Reinjectamos los tokens de inicio para la lectura de parámetros
            ts.unget(t2); // '('
            ts.unget(t);  // 'f'

            t = ts.get(); // name_token 'f'
            t2 = ts.get(); // char_token '('
            
            vector<string> params;

            // Leer parámetros: solo acepta NOMBRES aquí para una definición.
            Token p = ts.get();
            consumed_tokens.push_back(p);
            
            bool is_definition_syntax = false;

            if (p.kind == Token::id::name_token)
            {
                is_definition_syntax = true;
                params.push_back(p.name);

                Token sep = ts.get();
                consumed_tokens.push_back(sep);
                
                while (sep.is_symbol(','))
                {
                    Token pn = ts.get();
                    consumed_tokens.push_back(pn);
                    
                    if (pn.kind != Token::id::name_token)
                    {
                        is_definition_syntax = false;
                        break; 
                    }
                    params.push_back(pn.name);
                    
                    sep = ts.get();
                    consumed_tokens.push_back(sep);
                }

                if (!sep.is_symbol(')'))
                    is_definition_syntax = false;
            }
            else if (p.is_symbol(')'))
            {
                is_definition_syntax = true;
            }
            // Si es un número (como en f(1,2)), is_definition_syntax = false, lo cual es correcto.
            
            // Check for '='
            Token eq = ts.get();
            
            if (is_definition_syntax && eq.is_symbol('='))
            {
                // *** ES UNA DEFINICIÓN DE FUNCIÓN ***

                // Registrar función
                generic_functions[fname] = GenericFunction(fname, params, ""); // Inicializamos el cuerpo vacío
                ostringstream body;

                // Leer el cuerpo de la función hasta el ';'
                while (true)
                {
                    Token tk = ts.get();
                    if (tk.kind == Token::id::print) break; 

                    if (tk.kind == Token::id::number)
                    {
                        ostringstream ss; tk.value.to_stream(ss); body << ss.str();
                    } else if (tk.kind == Token::id::name_token) {
                        body << tk.name;
                    } else if (tk.kind == Token::id::char_token) {
                        body << tk.symbol;
                    } else {
                        throw runtime_error("bad token in function body");
                    }
                }
                
                // Actualizar el cuerpo de la función
                generic_functions[fname].body_expression = body.str();
                
                // (Feedback al usuario)
                cout << "Function " << fname << "(";
                for(size_t i=0; i<params.size(); ++i) { 
                    cout << params[i] << (i<params.size()-1 ? "," : ""); 
                }
                cout << ") defined.\n";

                // Limpieza del stream para asegurar que no hay residuos
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                return gv(0.0);
            }
            else
            {
                // *** ES UNA LLAMADA O ASIGNACIÓN NORMAL (Falló la definición) ***
                
                // 0. Devolver el token usado para chequear el '='
                ts.unget(eq);
                
                // 1. Devolver todos los tokens consumidos en la lectura de parámetros, en orden inverso.
                for (auto it = consumed_tokens.rbegin(); it != consumed_tokens.rend(); ++it)
                {
                    ts.unget(*it);
                }
                
                // 2. Devolver los tokens de apertura '(' y 'f'
                ts.unget(t2); // '('
                ts.unget(t);  // 'f'

                // Caer al flujo normal de expresión
                goto normal_statement;
            }
        }
        else
        {
            ts.unget(t2); // Devolver el token 'peeked' (ej: el '=')
        }
    }

    // 3. Lógica normal (const, asignación, expresión)
    
normal_statement:

    if(t.kind == Token::id::const_token)
        return constant_assign();

    if(t.kind == Token::id::name_token)
    {
        Token tt = ts.get();

        if(tt.is_symbol('='))
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

    ts.unget(t);
    return expression();
}