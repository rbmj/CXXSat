#include <iostream>
#include <CXXSat/Circuit.h>
#include <CXXSat/Argument.h>
#include <CXXSat/Variable.h>
#include <CXXSat/Sat.h>

int main() {
    auto c = Circuit();
    auto arg = c.addArgument<uint32_t>();
    auto x = arg.asValue();
    auto z = (x * 0x1234U) / 0x1234U;
    auto p = c.generateCNF(z == 0x1337U);
    std::cout << "CNF generated\n";
    //p.printDIMACS(std::cout);
    auto soln = p.solve();
    if (soln) {
        std::cout << "SAT\n";
        std::cout << "arg: ";
        //std::cout << std::hex;
        arg.print(std::cout, soln);
        std::cout << '\n';
    }
    else {
        std::cout << "UNSAT\n";
    }
    return 0;
}
