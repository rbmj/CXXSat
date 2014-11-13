#include <CXXSat/Variable.h>
#include <CXXSat/Circuit.h>
#include <CXXSat/DynVar.h>

//avoid ugly syntax...
#define CALL_MEMBER(obj, fun) ((obj).*(fun))

//explicit instantiation
template class IntVar<true, 8>;
template class IntVar<false, 8>;
template class IntVar<true, 16>;
template class IntVar<false, 16>;
template class IntVar<true, 32>;
template class IntVar<false, 32>;
template class IntVar<true, 64>;
template class IntVar<false, 64>;

std::unique_ptr<Variable> Variable::Not() const {
    return isZero().clone();
}

std::unique_ptr<Variable> Variable::LogAnd(const Variable& v) const {
    return ((const Variable&)(!(this->isZero()) & !(v.isZero()))).clone();
}

std::unique_ptr<Variable> Variable::LogOr(const Variable& v) const {
    return ((const Variable&)(!(this->isZero()) | !(v.isZero()))).clone();
}

BitVar::BitVar(const std::weak_ptr<Circuit::impl>& c) : Base(c) {}

BitVar::BitVar(const BitArgument& arg) : Base(arg.getCircuit(), BitArr{{Circuit::Value(*(arg.getInput()))}}) {}

BitVar::BitVar(const BitVar& v) : Base(v.getCircuit(), BitArr{{v.getBit()}}) {}

BitVar::BitVar(const Circuit::Value& v) : Base(v.getCircuit(), BitArr{{v}}) {}

BitVar::BitVar(bool b, const std::weak_ptr<Circuit::impl>& c) : Base(c, BitArr{{b ?
        Circuit::getLiteralTrue(c) : Circuit::getLiteralFalse(c)}}) {}

BitVar BitVar::FromDynamic(const DynVar& d) {
    if (d.isBit()) {
        return {(BitVar&)(*(d.var))};
    }
    else {
        auto x = d.asBool();
        assert(x.isBit());
        return {(BitVar&)(*(x.var))};
    }
}

Circuit::Value& BitVar::getBit() {
    return getBits().at(0);
}

const Circuit::Value& BitVar::getBit() const {
    return getBits().at(0);
}

BitVar& BitVar::operator=(const BitVar& v) {
    //TODO:  Assert circuits equal
    getBit() = v.getBit();
    return *this;
}

BitVar BitVar::Not(const BitVar& v) {
    return BitVar{::Not(v.getBit())};
}

BitVar BitVar::And(const BitVar& a, const BitVar& b) {
    return BitVar(::And(a.getBit(), b.getBit()));
}

BitVar BitVar::Nand(const BitVar& a, const BitVar& b) {
    return BitVar(::Nand(a.getBit(), b.getBit()));
}
BitVar BitVar::Or(const BitVar& a, const BitVar& b) {
    return BitVar(::Or(a.getBit(), b.getBit()));
}
BitVar BitVar::Nor(const BitVar& a, const BitVar& b) {
    return BitVar(::Nor(a.getBit(), b.getBit()));
}
BitVar BitVar::Xor(const BitVar& a, const BitVar& b) {
    return BitVar(::Xor(a.getBit(), b.getBit()));
}
BitVar BitVar::Xnor(const BitVar& a, const BitVar& b) {
    return BitVar(::Xnor(a.getBit(), b.getBit()));
}

BitVar BitVar::MultiAnd(const std::vector<BitVar>& vec) {
    std::vector<Circuit::Value> values;
    for (auto& var : vec) {
        values.push_back(var.getBit());
    }
    return BitVar(::MultiAnd(values));
}

BitVar BitVar::MultiOr(const std::vector<BitVar>& vec) {
    std::vector<Circuit::Value> values;
    for (auto& var : vec) {
        values.push_back(var.getBit());
    }
    return BitVar(::MultiOr(values));
}

int BitVar::getID() const {
    return getBit().getID();
}

std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Add(const Variable& v) const {
    return (IntVar<true, int_size>(CAST(*this)) + 
            IntVar<true, int_size>(CAST(v))).clone();
}
std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Sub(const Variable& v) const {
    return (IntVar<true, int_size>(CAST(*this)) -
            IntVar<true, int_size>(CAST(v))).clone();
}
std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Mul(const Variable& v) const {
    return (IntVar<true, int_size>(CAST(*this)) *
            IntVar<true, int_size>(CAST(v))).clone();
}
std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Shr(const Variable& v) const {
    return (IntVar<true, int_size>(CAST(*this)) >>
            IntVar<true, int_size>(CAST(v))).clone();
}
std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Shl(const Variable& v) const {
    return (IntVar<true, int_size>(CAST(*this)) <<
            IntVar<true, int_size>(CAST(v))).clone();
}
std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Shr(unsigned u) const {
    return (IntVar<true, int_size>(CAST(*this)) >> u).clone();
}
std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Shl(unsigned u) const {
    return (IntVar<true, int_size>(CAST(*this)) << u).clone();
}
std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Neg() const {
    return (~(IntVar<true, int_size>(CAST(*this)))).clone();
}

std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Less(const Variable& v) const {
    return (!CAST(*this) & CAST(v)).clone();
}

std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Equal(const Variable& v) const {
    return BitVar::Xnor(CAST(*this), CAST(v)).clone();
}

std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Minus() const {
    return IntVar<true, int_size>(CAST(*this)).operator-().clone();
}

std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Promote() const {
    return IntVar<true, int_size>(CAST(*this)).clone();
}

void Variable::Base<BitVar, 0>::DivMod(const Variable& d, std::unique_ptr<Variable>* qp, var_ptr* rp) const {
    IntVar<true, int_size> q(this->getCircuit()), r(this->getCircuit());
    IntVar<true, int_size>::DivRem({CAST(*this)}, {CAST(d)}, &q, &r);
    if (qp) *qp = q.clone();
    if (rp) *rp = r.clone();
}

std::unique_ptr<Variable> Variable::Base<BitVar, 0>::do_cast(int newtype) const {
    switch (newtype) {
    case -64:
        return std::make_unique<IntVar<true, 64>>((const BitVar&)*this);
    case -32:
        return std::make_unique<IntVar<true, 32>>((const BitVar&)*this);
    case -16:
        return std::make_unique<IntVar<true, 16>>((const BitVar&)*this);
    case -8:
        return std::make_unique<IntVar<true, 8>>((const BitVar&)*this);
    case 8:
        return std::make_unique<IntVar<false, 8>>((const BitVar&)*this);
    case 16:
        return std::make_unique<IntVar<false, 16>>((const BitVar&)*this);
    case 32:
        return std::make_unique<IntVar<false, 32>>((const BitVar&)*this);
    case 64:
        return std::make_unique<IntVar<false, 64>>((const BitVar&)*this);
    default:
        assert(false); //unimplemented
        break;
    }
    return nullptr; //can't happen
}

