#include "Variable.h"
#include "Circuit.h"

Variable::~Variable() {}

BitVar::BitVar(const BitArgument& arg) {
    getBits().push_back(Value::create(arg.getInputs().at(0)));
}

BitVar::BitVar(const BitVar& v) {
    getBits().push_back(v.getBits().at(0)->clone());
}

BitVar::BitVar(std::shared_ptr<Value> v) {
    getBits().push_back(std::move(v));
}

BitVar& BitVar::operator=(const BitVar& v) {
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
BitVar BitVar::Nxor(const BitVar& a, const BitVar& b) {
    return BitVar(::Nxor(a.getBits().at(0), b.getBits().at(0)));
}

int BitVar::getID() const {
    return getBits().at(0)->getID();
}

