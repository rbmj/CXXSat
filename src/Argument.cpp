#include <sstream>

#include <CXXSat/Argument.h>
#include <CXXSat/Circuit.h>
#include <CXXSat/Variable.h>

//explicit instantiation
template class IntArg<true, 8>;
template class IntArg<false, 8>;
template class IntArg<true, 16>;
template class IntArg<false, 16>;
template class IntArg<true, 32>;
template class IntArg<false, 32>;
template class IntArg<true, 64>;
template class IntArg<false, 64>;

std::string Argument::toString(const Solution& s) const {
    std::ostringstream ss;
    print(ss, s);
    return ss.str();
}

BitArgument::BitArgument(const std::weak_ptr<Circuit::impl>& c) : Argument(c) {
    getInputs().push_back(Circuit::Input::create(c));
}

BitVar BitArgument::asValue() const {
    return BitVar(*this);
}

int BitArgument::getID() const {
    return getInputs().at(0)->getID();
}

bool BitArgument::solution(const Solution& s) const {
    return s.at(getID());
}

DynVar BitArgument::asDynamic() const {
    return DynVar{asValue()};
}

void BitArgument::print(std::ostream& o, const Solution& s) const {
    o << solution(s);
}
