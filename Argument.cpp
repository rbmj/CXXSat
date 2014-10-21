#include "Argument.h"
#include "Circuit.h"
#include "Variable.h"

BitArgument::BitArgument(Circuit* c) : Argument() {
    getInputs().push_back(Input::create(c));
}

BitVar BitArgument::asValue() const {
    return BitVar(*this);
}

int BitArgument::getID() const {
    return getInputs().at(0)->getID();
}
