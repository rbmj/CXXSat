#include <iostream>
#include <CXXSat/Circuit.h>
#include <CXXSat/Argument.h>
#include <CXXSat/Variable.h>
#include <CXXSat/Sat.h>


int main() {
    auto c = Circuit();
    auto arg = c.addArgument<UIntArg32>();
    auto x = arg->asValue();
    auto a = c.getLiteral<UIntVar32>(0x1234);
    auto y = a * x;
    auto z = y / a;
    //auto key = c.getLiteral<UIntVar32>(0x12345678);
    //auto y = x ^ key;
    auto val = c.getLiteral<UIntVar32>(0x1337);
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
