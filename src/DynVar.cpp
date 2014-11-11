#include <CXXSat/DynVar.h>
#include <CXXSat/Variable.h>

std::unique_ptr<Variable> DynVar::binary_operation(Variable::op_t op, Variable* a, Variable* b) {
    if (a->isBit() && b->isBit()) {
        return CALL_MEMBER(*a, op)(*b);
    }
    unsigned op_size = std::max(a->size(), b->size());
    bool op_sign;
    if (a->sign() == b->sign()) {
        op_sign = a->sign();
    }
    else {
        //signedness differs
        if (a->size() == b->size()) {
            op_sign = false; //unsigned prevails if sizes equal
        }
        else {
            op_sign = ((a->size() > b->size()) ? a : b)->sign();
        }
    }
    //however, all of that said:
    if (op_size < size_int) {
        //all values are converted to int
        op_size = size_int;
        op_sign = true;
    }
    auto new_a = a->do_cast(op_sign, op_size);
    auto new_b = b->do_cast(op_sign, op_size);
    return CALL_MEMBER(*new_a, op)(*new_b);
}
    
DynVar::DynVar(const Variable& v) : var(v.clone()) {}
DynVar::DynVar(std::unique_ptr<Variable>&& v) : var(std::move(v)) {}
DynVar::DynVar(const DynVar& v) : var(v.var->clone()) {}
DynVar::DynVar(DynVar&& v) : var(std::move(v.var)) {}

bool DynVar::isSigned() const;
unsigned DynVar::numBits() const;
bool DynVar::isBit() const;
DynVar DynVar::operator!() const;
DynVar DynVar::operator~() const;
DynVar DynVar::operator-() const;
DynVar DynVar::operator+() const;
DynVar DynVar::operator&(const DynVar&) const;
DynVar DynVar::operator|(const DynVar&) const;
DynVar DynVar::operator^(const DynVar&) const;
DynVar DynVar::operator+(const DynVar&) const;
DynVar DynVar::operator-(const DynVar&) const;
DynVar DynVar::operator*(const DynVar&) const;
DynVar DynVar::operator/(const DynVar&) const;
DynVar DynVar::operator%(const DynVar&) const;
DynVar DynVar::operator<<(const DynVar&) const;
DynVar DynVar::operator>>(const DynVar&) const;
DynVar DynVar::operator==(const DynVar&) const;
DynVar DynVar::operator!=(const DynVar&) const;
DynVar DynVar::operator<(const DynVar&) const;
DynVar DynVar::operator>(const DynVar&) const;
DynVar DynVar::operator<=(const DynVar&) const;
DynVar DynVar::operator>=(const DynVar&) const;
DynVar DynVar::operator<<(unsigned u) const;
DynVar DynVar::operator>>(unsigned u) const;

//implement here so that the unique_ptr will compile
DynVar::~DynVar() {}
