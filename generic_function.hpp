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
        std::vector<std::string> params;
        std::string body_expression;

        GenericFunction() {}

        GenericFunction(const std::string& n,
                    const std::vector<std::string>& p,
                    const std::string& expr)
            : name(n), params(p), body_expression(expr)
        {}

        size_t arity() const { return params.size(); }
    };

    extern std::map<std::string, GenericFunction> generic_functions;

    gv call_generic_function(const GenericFunction& f,
                        const std::vector<gv>& args);

#endif
