#include <iostream>
#include "calculate.hpp"
#include "token_stream.hpp"

Token_stream ts;
int precision = 6;

int main() {
    help();
    calculate();
    return 0;
}
