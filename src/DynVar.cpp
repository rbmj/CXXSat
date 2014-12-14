#include <CXXSat/DynVar.h>
#include <CXXSat/Variable.h>

//avoid ugly syntax...
#define CALL_MEMBER(obj, fun) ((obj).*(fun))

DynVar DynVar::binary_operation(op_t op, const Variable& a, const Variable& b) {
    if (a.isBit() && b.isBit()) {
        return DynVar{CALL_MEMBER(a, op)(b)};
    }
    unsigned op_size = std::max(a.size(), b.size());
    bool op_sign;
    if (a.sign() == b.sign()) {
        op_sign = a.sign();
    }
    else {
        //signedness differs
        if (a.size() == b.size()) {
            op_sign = false; //unsigned prevails if sizes equal
        }
        else {
            op_sign = ((a.size() > b.size()) ? a : b).sign();
        }
    }
    //however, all of that said:
    if (op_size < Variable::int_size) {
        //all values are converted to int
        op_size = Variable::int_size;
        op_sign = true;
    }
    auto type = getType(op_sign, op_size);
    auto new_a = a.do_cast(type);
    auto new_b = b.do_cast(type);
    return DynVar{CALL_MEMBER(*new_a, op)(*new_b)};
}

DynVar DynVar::do_cast(int type) const {
    return DynVar{var->do_cast(type)};
}

DynVar DynVar::mask(const BitVar& cond) const {
    return DynVar{var->Mask(cond)};
}

DynVar DynVar::mask(const DynVar& cond) const {
    return mask(BitVar::FromDynamic(cond));
}

DynVar DynVar::Ternary(const DynVar& cond, const DynVar& true_val, const DynVar& false_val) {
    auto condbit = BitVar::FromDynamic(cond);
    return true_val.mask(condbit) | false_val.mask(!condbit);
}

DynVar::DynVar(const Variable& v) : var(v.clone()) {}
DynVar::DynVar(std::unique_ptr<Variable>&& v) : var(std::move(v)) {}
DynVar::DynVar(const DynVar& v) : var(v.var->clone()) {}
DynVar::DynVar(DynVar&& v) : var(std::move(v.var)) {}
DynVar& DynVar::operator=(const DynVar& v) {
    var = v.var->do_cast(getType(var->sign(), var->size()));
    return *this;
}

bool DynVar::isSigned() const {
    return var->sign();
}

unsigned DynVar::numBits() const {
    return var->size();
}

bool DynVar::isBit() const {
    return var->isBit();
}

DynVar DynVar::isZero() const {
    return DynVar{var->isZero()};
}

DynVar DynVar::asBool() const {
    return DynVar{!(var->isZero())};
}

DynVar DynVar::operator!() const {
    return DynVar{var->Not()};
}

DynVar DynVar::operator~() const {
    return DynVar{var->Neg()};
}

DynVar DynVar::operator-() const {
    return DynVar{var->Minus()};
}

DynVar DynVar::operator+() const {
    return DynVar{var->Promote()};
}

DynVar DynVar::operator&&(const DynVar& v) const {
    return DynVar{var->LogAnd(*(v.var))};
}

DynVar DynVar::operator||(const DynVar& v) const {
    return DynVar{var->LogOr(*(v.var))};
}

DynVar DynVar::operator&(const DynVar& v) const {
    return binary_operation(&Variable::And, *var, *(v.var));
}

DynVar DynVar::operator|(const DynVar& v) const {
    return binary_operation(&Variable::Or, *var, *(v.var));
}

DynVar DynVar::operator^(const DynVar& v) const {
    return binary_operation(&Variable::Xor, *var, *(v.var));
}

DynVar DynVar::operator+(const DynVar& v) const {
    return binary_operation(&Variable::Add, *var, *(v.var));
}

DynVar DynVar::operator-(const DynVar& v) const {
    return binary_operation(&Variable::Sub, *var, *(v.var));
}

DynVar DynVar::operator*(const DynVar& v) const {
    return binary_operation(&Variable::Mul, *var, *(v.var));
}

DynVar DynVar::operator/(const DynVar& v) const {
    return binary_operation(&Variable::Div, *var, *(v.var));
}

DynVar DynVar::operator%(const DynVar& v) const {
    return binary_operation(&Variable::Mod, *var, *(v.var));
}

DynVar DynVar::operator<<(const DynVar& v) const {
    return binary_operation(&Variable::Shl, *var, *(v.var));
}

DynVar DynVar::operator>>(const DynVar& v) const {
    return binary_operation(&Variable::Shr, *var, *(v.var));
}

DynVar DynVar::operator==(const DynVar& v) const {
    return binary_operation(&Variable::Equal, *var, *(v.var));
}

DynVar DynVar::operator!=(const DynVar& v) const {
    return binary_operation(&Variable::NotEq, *var, *(v.var));
}

DynVar DynVar::operator<(const DynVar& v) const {
    return binary_operation(&Variable::Less, *var, *(v.var));
}

DynVar DynVar::operator>(const DynVar& v) const {
    return binary_operation(&Variable::Greater, *var, *(v.var));
}

DynVar DynVar::operator<=(const DynVar& v) const {
    return binary_operation(&Variable::LessEq, *var, *(v.var));
}

DynVar DynVar::operator>=(const DynVar& v) const {
    return binary_operation(&Variable::GreaterEq, *var, *(v.var));
}

DynVar DynVar::operator<<(unsigned u) const {
    return DynVar{(+(*this)).var->Shl(u)};
}

DynVar DynVar::operator>>(unsigned u) const {
    return DynVar{(+(*this)).var->Shr(u)};
}

template <>
DynVar DynVar::cast<bool>() const {
    return do_cast(0);
}


//implement here so that the unique_ptr will compile
DynVar::~DynVar() {}
