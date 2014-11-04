#include <iostream>
#include "../Circuit.h"
#include "../Argument.h"
#include "../Variable.h"
#include "../Sat.h"

int main() {
    auto c = Circuit();
    auto arg = c.addArgument<UIntArg32>();
    auto x = arg->asValue();
    auto key = c.getLiteral<UIntVar32>(0x12345678);
    auto y = x ^ key;
    c.yield(y);
    c.constrain_equal<UIntVar32>(0x41414141);
    auto p = c.generateCNF();
    auto soln = p.solve();
    if (soln) {
        std::cout << "SAT\n";
        std::cout << "arg: ";
        std::cout << std::hex;
        arg->print(std::cout, soln);
        std::cout << '\n';
    }
    return 0;
}
