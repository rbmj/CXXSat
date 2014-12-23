#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <CXXSat/Circuit.h>
#include <CXXSat/Argument.h>
#include <CXXSat/Variable.h>
#include <CXXSat/Sat.h>

int main(int argc, char** argv) {
    CastMode::set(CastMode::MANUAL);
    auto c = Circuit();
    if (argc < 3 || argc > 4 || (argc == 4 && argv[3] != std::string("-dump"))) {
        std::cerr << "USAGE: " + std::string(argv[0]) + " numbits prime [-dump]\n";
        return 1;
    }
    int n = atoi(argv[1]);
    auto x_arg = c.addArgument(TypeInfo{false, n/2});
    auto x = x_arg.asValue();
    auto y_arg = c.addArgument(TypeInfo{false, n/2});
    auto y = y_arg.asValue();
    auto z = c.getLiteral(FlexInt::fromString(argv[2], TypeInfo{false, n}));
    auto p = c.generateCNF(z == Variable::Mul_full(x, y));
    if (argc == 4) {
        p.printDIMACS(std::cout);
    }
    else {
        auto soln = p.solve();
        if (soln) {
            std::cout << x_arg.solution(soln) << ' ' << y_arg.solution(soln) << '\n';
        }
        else {
            std::cerr << "UNSAT\n";
            return 1;
        }
    }
    return 0;
}
