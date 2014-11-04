#include "Variable.h"
#include "Circuit.h"

//explicit instantiation
template class std::vector<std::shared_ptr<Circuit::Value>>;
template class IntVar<true, 8>;
template class IntVar<false, 8>;
template class IntVar<true, 16>;
template class IntVar<false, 16>;
template class IntVar<true, 32>;
template class IntVar<false, 32>;
template class IntVar<true, 64>;
template class IntVar<false, 64>;

int Variable::class_id = 0;

BitVar Variable::operator!=(const Variable& v) const {
    return !(*this == v);
}

template <class Derived>
BitVar Variable::Base<Derived>::operator==(const Variable& v) const {
    assert(getTypeID() == v.getTypeID());
    return *this == (const Derived&)v;
}

BitVar::BitVar(const BitArgument& arg) : Variable::Base<BitVar>(arg.getCircuit()) {
    getBits().push_back(Circuit::Value::create(arg.getInputs().at(0)));
}

BitVar::BitVar(const BitVar& v) : Variable::Base<BitVar>(v.getCircuit()) {
    getBits().push_back(v.getBits().at(0)->clone());
}

BitVar::BitVar(std::shared_ptr<Circuit::Value> v) : Variable::Base<BitVar>(v->getCircuit()) {
    getBits().push_back(std::move(v));
}

BitVar::BitVar(bool b, const std::weak_ptr<Circuit::impl>& c) : Variable::Base<BitVar>(c) {
    getBits().push_back(b ? Circuit::getLiteralTrue(c) : Circuit::getLiteralFalse(c));
}

value_ptr BitVar::getBit() const {
    return getBits().at(0);
}

BitVar& BitVar::operator=(const BitVar& v) {
    //TODO:  Assert circuits equal
    getBits().at(0) = v.getBits().at(0)->clone();
    return *this;
}

BitVar BitVar::Not(BitVar v) {
    v.getBits().at(0) = ::Not(v.getBits().at(0));
    return std::move(v);
}

BitVar BitVar::And(const BitVar& a, const BitVar& b) {
    return BitVar(::And(a.getBits().at(0), b.getBits().at(0)));
}
    
BitVar BitVar::Nand(const BitVar& a, const BitVar& b) {
    return BitVar(::Nand(a.getBits().at(0), b.getBits().at(0)));
}
BitVar BitVar::Or(const BitVar& a, const BitVar& b) {
    return BitVar(::Or(a.getBits().at(0), b.getBits().at(0)));
}
BitVar BitVar::Nor(const BitVar& a, const BitVar& b) {
    return BitVar(::Nor(a.getBits().at(0), b.getBits().at(0)));
}
BitVar BitVar::Xor(const BitVar& a, const BitVar& b) {
    return BitVar(::Xor(a.getBits().at(0), b.getBits().at(0)));
}
BitVar BitVar::Xnor(const BitVar& a, const BitVar& b) {
    return BitVar(::Xnor(a.getBits().at(0), b.getBits().at(0)));
}

int BitVar::getID() const {
    return getBits().at(0)->getID();
}

