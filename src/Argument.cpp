#include <sstream>

#include <CXXSat/Argument.h>
#include <CXXSat/Circuit.h>
#include <CXXSat/Variable.h>

Variable Argument::asValue() const {
    return Variable{*this};
}

std::string Argument::toString(const Solution& s) const {
    std::ostringstream ss;
    print(ss, s);
    return ss.str();
}

FlexInt Argument::solution(const Solution& s) const {
    FlexInt t{0, getTypeInfo()};
    FlexInt one{1, getTypeInfo()};
    auto& inputs = getInputs();
    for (unsigned i = 0; i < size(); ++i) {
        int id = inputs[i]->getID();
        if (id != 0 && s.at(id)) {
            t |= one << i;
        }
    }
    return t;
}

void Argument::print(std::ostream& o, const Solution& s) const {
    o << +solution(s);
}

