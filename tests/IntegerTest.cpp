#include <iostream>
#include <CXXSat/Circuit.h>
#include <CXXSat/Argument.h>
#include <CXXSat/Variable.h>
#include <CXXSat/Sat.h>


int main() {
    auto c = Circuit();
    auto arg = c.addArgument<IntArg32>();
    auto x = arg->asValue();
    auto z = IntVar32::Ternary((x == 5), x, x - x);
    auto val = c.getLiteral<IntVar32>(5);
    auto p = c.generateCNF(z == val);
    std::cout << "CNF generated\n";
    auto soln = p.solve();
    if (soln) {
        std::cout << "SAT\n";
        std::cout << "arg: ";
        //std::cout << std::hex;
        arg->print(std::cout, soln);
        std::cout << '\n';
    }
    else {
        std::cout << "UNSAT\n";
    }
    return 0;
}
