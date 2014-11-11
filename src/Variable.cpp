#include <CXXSat/Variable.h>
#include <CXXSat/Circuit.h>

//avoid ugly syntax...
#define CALL_MEMBER(obj, fun) ((obj).*(fun))

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

BitVar::BitVar(const std::weak_ptr<Circuit::impl>& c) : Base(c) {}

BitVar::BitVar(const BitArgument& arg) : Base(arg.getCircuit()) {
    getBits().push_back(Circuit::Value::create(arg.getInputs().at(0)));
}

BitVar::BitVar(const BitVar& v) : Base(v.getCircuit()) {
    getBits().push_back(v.getBits().at(0)->clone());
}

BitVar::BitVar(std::shared_ptr<Circuit::Value> v) : Base(v->getCircuit()) {
    getBits().push_back(std::move(v));
}

BitVar::BitVar(bool b, const std::weak_ptr<Circuit::impl>& c) : Base(c) {
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

BitVar BitVar::MultiAnd(const std::vector<BitVar>& vec) {
    std::vector<value_ptr> values;
    for (auto& var : vec) {
        values.push_back(var.getBits()[0]);
    }
    return BitVar(::MultiAnd(values));
}

BitVar BitVar::MultiOr(const std::vector<BitVar>& vec) {
    std::vector<value_ptr> values;
    for (auto& var : vec) {
        values.push_back(var.getBits()[0]);
    }
    return BitVar(::MultiOr(values));
}

int BitVar::getID() const {
    return getBits().at(0)->getID();
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
std::unique_ptr<Variable> Variable::Base<BitVar, 0>::Neg() const {
    return (~(IntVar<true, int_size>(CAST(*this)))).clone();
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

