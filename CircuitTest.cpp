#include <iostream>
#include "Circuit.h"
#include "Argument.h"
#include "Variable.h"

int main() {
    auto c = Circuit();
    auto x1_arg = c.addArgument<BitArgument>("x1");
    auto x2_arg = c.addArgument<BitArgument>("x2");
    auto x3_arg = c.addArgument<BitArgument>("x3");
    auto x1 = x1_arg->asValue();
    auto x2 = x2_arg->asValue();
    auto x3 = x3_arg->asValue();
    auto y = (!x1 & x2) | (x1 & !x2) | (!x2 & x3);
    c.number();
    std::cout << "x1: " << x1_arg->getID() << '\n';
    std::cout << "x2: " << x2_arg->getID() << '\n';
    std::cout << "x3: " << x3_arg->getID() << '\n';
    std::cout << "y: " << y.getID() << '\n';
    printDIMACS(c.generateCNF());
    return 0;
}
