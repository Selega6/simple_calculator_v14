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
// Archivo: parser.cpp

// ...
static std::string replace_parameters(
    const GenericFunction& f,
    const std::vector<gv>& args)
{
    // [DEBUG] Inicio de la sustitución de parámetros
    std::cerr << "DEBUG: replace_parameters: Function '" << f.name << "'. Arity=" << f.params.size() << ". Args received=" << args.size() << ".\n"; 

    if(args.size() != f.params.size())
        throw runtime_error("wrong number of arguments for function " + f.name);

    std::string expr = f.body_expression;
    std::cerr << "DEBUG: replace_parameters: Original body: '" << expr << "'.\n"; // [DEBUG] Muestra el cuerpo original

    for(size_t i = 0; i < f.params.size(); ++i)
    {
        std::ostringstream ss;
        // Asumiendo que gv::to_stream es el método para convertir a string
        args[i].to_stream(ss); 
        std::string arg_value_str = ss.str(); // Capturamos el valor como string
        
        std::cerr << "DEBUG: replace_parameters: Replacing param '" << f.params[i] << "' with value '" << arg_value_str << "'.\n"; // [DEBUG] Muestra la sustitución

        size_t pos = 0;
        // Reemplaza todas las ocurrencias del nombre del parámetro en la expresión
        while((pos = expr.find(f.params[i], pos)) != std::string::npos)
        {
            expr.replace(pos, f.params[i].size(), arg_value_str);
            pos += arg_value_str.size();
        }
    }
    
    std::cerr << "DEBUG: replace_parameters: Final processed expression: '" << expr << "'.\n"; // [DEBUG] Muestra la expresión final a evaluar
    return expr;
}
// ...
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
    std::cerr << "DEBUG: call_generic_function: START execution for '" << f.name << "'.\n"; // [DEBUG] Inicio de la llamada

    // 1. Reemplazar parámetros y preparar la expresión para la evaluación.
    std::string processed = replace_parameters(f, args);
    
    std::istringstream iss(processed);
    
    // CORRECCIÓN CLAVE: Forzar el locale clásico (C) para asegurar el punto decimal.
    iss.imbue(std::locale::classic()); 
    
    std::cerr << "DEBUG: call_generic_function: Starting internal parser on stream for: '" << processed << "'.\n"; // [DEBUG] Se confirma la expresión a evaluar

    // 2. Guardar y Redirigir la Entrada Estándar (std::cin)
    std::streambuf* oldbuf = std::cin.rdbuf(iss.rdbuf());

    // 3. Guardar y Reiniciar el Token_stream Global
    extern Token_stream ts;
    Token_stream ts_backup = ts;
    ts = Token_stream(); // reiniciar temporalmente

    gv result;
    try {
        // Ejecutar el parser recursivamente
        result = expression(); 
        
        std::cerr << "DEBUG: call_generic_function: Internal parser result received.\n"; // [DEBUG] Resultado del parser interno

    } catch (...) {
        // 4. Restaurar el estado en caso de error
        std::cin.rdbuf(oldbuf);
        ts = ts_backup;
        std::cerr << "DEBUG: call_generic_function: ERROR during internal parsing. State restored.\n"; // [DEBUG] Error y restauración
        throw;
    }

    // 5. Restaurar cin y el Token_stream global
    std::cin.rdbuf(oldbuf);
    ts = ts_backup;
    
    std::cerr << "DEBUG: call_generic_function: END execution for '" << f.name << "'. State restored.\n"; // [DEBUG] Fin y restauración

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
// ---------------------------------------------
//  PRIMARY (CORREGIDA para manejar llamadas a función)
// ---------------------------------------------
gv primary()
{
    Token t = ts.get();
    
    // [DEBUG] Mostrar el token que Primary intenta procesar.
    std::cerr << "DEBUG: primary: Consumed token kind: " << t.value 
              << ", symbol: " << t.symbol 
              << ", name: " << t.name << "\n"; 

    // 1. Funciones internas (sin, cos, etc.)
    if (t.is_function()) 
    {
        std::cerr << "DEBUG: primary: Identified built-in function: " << t.name << ". Calling function_name().\n"; // [DEBUG]
        ts.unget(t);
        return function_name();
    }

    // 2. Nombre: Variable o Llamada a Función Genérica
    if (t.kind == Token::id::name_token)
    {
        string fname = t.name;
        std::cerr << "DEBUG: primary: Potential name/function: " << fname << "\n"; // [DEBUG]
        Token t2 = ts.get(); // Peek ahead for '('

        if (t2.is_symbol('(')) // ES UNA LLAMADA A FUNCIÓN GENÉRICA: f(1,2)
        {
            std::cerr << "DEBUG: primary: Identified Function Call: " << fname << "().\n"; // [DEBUG]
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
                    std::cerr << "DEBUG: primary: Parsed argument: " << args.back() << "\n"; // [DEBUG] Muestra el argumento parseado
                    Token sep = ts.get();
                    
                    if (sep.is_symbol(')')) break; // Fin de la lista de argumentos
                    
                    if (!sep.is_symbol(',')) 
                    {
                        std::cerr << "DEBUG: primary: ERROR: Expected ',' or ')' in function call arguments. Found kind: " << sep.kind << "\n"; // [DEBUG]
                        throw runtime_error("',' or ')' expected in function call");
                    }
                    
                    // Si es ',', continuamos el ciclo.
                }
            }
            
            std::cerr << "DEBUG: primary: Function call finished parsing arguments. Total args: " << args.size() << "\n"; // [DEBUG]
            
            if (generic_functions.count(fname) == 0)
            {
                std::cerr << "DEBUG: primary: ERROR: Undefined function: " << fname << "\n"; // [DEBUG]
                throw runtime_error("undefined function: " + fname);
            }

            std::cerr << "DEBUG: primary: Calling call_generic_function for: " << fname << " with " << args.size() << " arguments.\n"; // [DEBUG]

            return call_generic_function(generic_functions[fname], args);
        }

        // Si no es '(', es una variable. Devolvemos el token peeked.
        ts.unget(t2); 

        // 3. Es una Variable o Constante
        std::cerr << "DEBUG: primary: Reading value for variable/constant: " << fname << "\n"; // [DEBUG]
        return get_value(fname); 
    }
    
    
    if (t.kind == Token::id::number)
    {
        std::cerr << "DEBUG: primary: Identified number: " << t.value << "\n"; // [DEBUG]
        return t.value;
    }
    
    // Final fallback
    std::cerr << "DEBUG: primary: ERROR: Falling through, unexpected token kind: " << t.kind << "\n"; // [DEBUG]
    throw runtime_error("primary expected");
}


// ... (El resto de funciones del parser) ...

// Función auxiliar para leer el cuerpo de una función hasta el delimitador ';'
static std::string read_function_body()
{
    std::ostringstream body;
    std::string body_str;
    
    std::cerr << "DEBUG: read_function_body: Starting to read function body (until ';').\n"; // [DEBUG]

    while (true)
    {
        Token tk = ts.get();

        if (tk.kind == Token::id::print) // El token ';' es de tipo 'print'
        {
            std::cerr << "DEBUG: read_function_body: Delimiter ';' found. Body extraction finished.\n"; // [DEBUG]
            break; 
        }

        if (tk.kind == Token::id::quit)
            throw runtime_error("missing ';' after function body");

        // Convertir el token a su representación de cadena para el cuerpo
        if (tk.kind == Token::id::number)
        {
            std::ostringstream ss; 
            tk.value.to_stream(ss);
            body << ss.str();
        } 
        else if (tk.kind == Token::id::name_token) 
        {
            body << tk.name;
        } 
        else if (tk.kind == Token::id::char_token) 
        {
            body << tk.symbol;
        } 
        else 
        {
            std::cerr << "DEBUG: read_function_body: ERROR: Bad token value " << tk.value << " in function body.\n"; // [DEBUG]
            throw runtime_error("bad token in function body: expected expression");
        }
    }
    
    body_str = body.str();
    std::cerr << "DEBUG: read_function_body: Final body string: '" << body_str << "'.\n"; // [DEBUG]
    return body_str;
}


// ---------------------------------------------
//  STATEMENT (Estabilizado para Flujo de Tokens)
// ---------------------------------------------
gv statement()
{
    // 1. Obtener el primer token
    Token t = ts.get();
    string fname; 

    // 2. Caso 'const'
    if(t.kind == Token::id::const_token)
    {
        std::cerr << "DEBUG: statement: Identified CONST declaration.\n"; // [DEBUG]
        return constant_assign();
    }
    
    // 3. Posible Definición de Función
    if(t.kind == Token::id::name_token)
    {
        fname = t.name;
        
        Token t_open_paren = ts.get();
        if(t_open_paren.is_symbol('('))
        {
            // --- INICIO: LÓGICA DE DEFINICIÓN ---
            std::cerr << "DEBUG: statement: Name '" << fname << "' followed by '('. Potential Definition/Call.\n"; // [DEBUG]

            std::vector<Token> consumed_tokens;
            consumed_tokens.push_back(t_open_paren); // Guardar '('
            
            // Collect parameters/tokens inside parenthesis
            Token p = ts.get();
            consumed_tokens.push_back(p);
            
            std::vector<std::string> params;
            bool is_definition_candidate = true;

            if (p.kind == Token::id::name_token) 
            {
                params.push_back(p.name);
                Token sep = ts.get();
                consumed_tokens.push_back(sep);
                
                while (sep.is_symbol(','))
                {
                    Token pn = ts.get();
                    consumed_tokens.push_back(pn);
                    
                    if (pn.kind != Token::id::name_token) { is_definition_candidate = false; break; }
                    params.push_back(pn.name);
                    
                    sep = ts.get();
                    consumed_tokens.push_back(sep);
                }
                if (is_definition_candidate && !sep.is_symbol(')'))
                    is_definition_candidate = false; // Faltó ')' final
            }
            else if (!p.is_symbol(')')) // Si no es un nombre, y no es ')', es una expresión/valor (f(1))
            {
                // El primer token no es un nombre ni un ')' de cierre -> No es una definición
                is_definition_candidate = false; 
            }
            
            // Check for '='
            Token eq = ts.get();
            
            if (is_definition_candidate && eq.is_symbol('='))
            {
                // *** DEFINITION SUCCESS ***
                std::cerr << "DEBUG: statement: Confirmed Function Definition: " << fname << "\n"; // [DEBUG]
                std::string body_expr = read_function_body();
                generic_functions[fname] = GenericFunction(fname, params, body_expr);
                
                // (Feedback al usuario)
                std::cout << "Function " << fname << "(";
                for(size_t i=0; i<params.size(); ++i) { 
                    std::cout << params[i] << (i<params.size()-1 ? "," : ""); 
                }
                std::cout << ") defined.\n";
                return gv(0.0);
            }
            else
            {
                // *** DEFINITION FAILURE: Rewind ALL tokens ***
                std::cerr << "DEBUG: statement: Failed Definition Check. Rewinding tokens.\n"; // [DEBUG]
                
                // 0. Devolver el token usado para chequear el '='
                std::cerr << "DEBUG: statement: Rewinding token eq: kind=" << eq.value << ", symbol=" << eq.symbol << "\n"; // [DEBUG]
                ts.unget(eq);
                
                // 1. Devolver todos los tokens consumidos en la lectura de parámetros y '('
                for (auto it = consumed_tokens.rbegin(); it != consumed_tokens.rend(); ++it)
                {
                    std::cerr << "DEBUG: statement: Rewinding consumed token: kind=" << it->kind << ", symbol=" << it->symbol << ", name=" << it->name << "\n"; // [DEBUG]
                    ts.unget(*it);
                }
                
                // 2. Devolver el token de nombre ('f')
                std::cerr << "DEBUG: statement: Rewinding name token: " << fname << "\n"; // [DEBUG]
                ts.unget(t); 
            }
            // --- FIN: LÓGICA DE DEFINICIÓN ---
        }
        else // Name NOT followed by '(' (e.g. 'a = 3' o 'a + 3')
        {
            // Devolver el token que sigue a 'f' (ej: '=')
            ts.unget(t_open_paren);
            // Devolver el token de nombre ('f')
            ts.unget(t);
        }
    }

    // 4. Lógica de Asignación / Expresión (Unified flow)
    
    Token tt = ts.get(); // Lee el primer token (el nombre 'f' o 'a' o un número)

    if(tt.kind == Token::id::name_token)
    {
        // Se encontró un nombre. Chequeo de asignación: 'name = expression'
        Token eq_check = ts.get();
        if(eq_check.is_symbol('='))
        {
            std::cerr << "DEBUG: statement: Identified Assignment.\n"; // [DEBUG]
            ts.unget(eq_check);
            ts.unget(tt);
            return assign();
        }
        else
        {
            // Es una Expresión o Llamada a Función (e.g. f(1,2) o a+b)
            ts.unget(eq_check);
            ts.unget(tt);
            std::cerr << "DEBUG: statement: Identified Expression/Function Call.\n"; // [DEBUG]
            return expression();
        }
    }
    
    // Si no es 'const' ni 'name', es una expresión pura (e.g., 2+2)
    ts.unget(tt); 
    std::cerr << "DEBUG: statement: Identified Pure Expression.\n"; // [DEBUG]
    return expression();
}