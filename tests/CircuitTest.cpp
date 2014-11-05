#include <iostream>
#include <CXXSat/Circuit.h>
#include <CXXSat/Argument.h>
#include <CXXSat/Variable.h>
#include <CXXSat/Sat.h>

int main() {
    auto c = Circuit();
    auto x1_arg = c.addArgument<BitArgument>();
    auto x2_arg = c.addArgument<BitArgument>();
    auto x3_arg = c.addArgument<BitArgument>();
    auto x1 = x1_arg->asValue();
    auto x2 = x2_arg->asValue();
    auto x3 = x3_arg->asValue();
    //auto y = (!x1 & x2) | (x1 & !x2) | (!x2 & x3);
    auto y = BitVar::MultiOr({x1, x2, x3});
    c.yield(y);
    c.constrain_equal(true);
    /*
    std::cout << "x1: " << x1_arg->getID() << '\n';
    std::cout << "x2: " << x2_arg->getID() << '\n';
    std::cout << "x3: " << x3_arg->getID() << '\n';
    std::cout << "y: " << y.getID() << '\n';
    */
    auto p = c.generateCNF();
    //p.printDIMACS(std::cout);
    auto soln = p.solve();
    if (soln) {
        std::cout << "SAT\n";
        std::cout << "x1: ";
        x1_arg->print(std::cout, soln);
        std::cout << "\nx2: ";
        x2_arg->print(std::cout, soln);
        std::cout << "\nx3: ";
        x3_arg->print(std::cout, soln);
        std::cout << '\n';
    }
    return 0;
}
