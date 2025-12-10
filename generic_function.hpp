//crear una clase que trate con funciones generales#
#ifndef GENERIC_FUNCTION_HPP
    #define GENERIC_FUNCTION_HPP

    #include <string>
    #include <vector>
    #include <map>
    #include <stdexcept>
    #include <sstream>
    #include "generic_value.hpp"

    using gv = generic_value<double>;

    class GenericFunction
    {
    public:
        std::string name;
        std::vector<std::string> params;   // ["x","y"]
        std::string body_expression;       // "x+y"

        GenericFunction() {}

        GenericFunction(const std::string& n,
                    const std::vector<std::string>& p,
                    const std::string& expr)
            : name(n), params(p), body_expression(expr)
        {}

        size_t arity() const { return params.size(); }
    };

    /*
    * Mapa global donde guardaremos las funciones definidas por usuario.
    */
    extern std::map<std::string, GenericFunction> generic_functions;

    /*
    * Evalúa la función f(x,y) sustituyendo parámetros por valores,
    * y haciendo que el parser normal procese la expresión resultante.
    */
    gv call_generic_function(const GenericFunction& f,
                        const std::vector<gv>& args);

    
    //Para depurar:
    
    //std::string replace_parameters(const GenericFunction& f,
    //                            const std::vector<gv>& args);
#endif
