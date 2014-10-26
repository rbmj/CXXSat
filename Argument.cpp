#include "Argument.h"
#include "Circuit.h"
#include "Variable.h"

//explicit instantiation
template class IntArg<true, 8>;
template class IntArg<false, 8>;
template class IntArg<true, 16>;
template class IntArg<false, 16>;
template class IntArg<true, 32>;
template class IntArg<false, 32>;
template class IntArg<true, 64>;
template class IntArg<false, 64>;

BitArgument::BitArgument(const std::weak_ptr<Circuit::impl>& c) : Argument() {
    getInputs().push_back(Circuit::Input::create(c));
}

BitVar BitArgument::asValue() const {
    return BitVar(*this);
}

int BitArgument::getID() const {
    return getInputs().at(0)->getID();
}
