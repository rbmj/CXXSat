#include <CXXSat/DynArg.h>
#include <CXXSat/Argument.h>

DynVar DynArg::asValue() const {
    return arg->asDynamic();
}

void DynArg::print(std::ostream& o, const Solution& s) const {
    return arg->print(o, s);
}

std::string DynArg::toString(const Solution& s) const {
    return arg->toString(s);
}

