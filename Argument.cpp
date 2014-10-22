#include "Argument.h"
#include "Circuit.h"
#include "Variable.h"

BitArgument::BitArgument(const std::weak_ptr<Circuit::impl>& c) : Argument() {
    getInputs().push_back(Circuit::Input::create(c));
}

BitVar BitArgument::asValue() const {
    return BitVar(*this);
}

int BitArgument::getID() const {
    return getInputs().at(0)->getID();
}
