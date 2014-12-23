#include <CXXSat/Variable.h>
#include <CXXSat/Circuit.h>
#include <CXXSat/Argument.h>

CastMode::mode_t CastMode::mode = CastMode::C_STYLE;

Variable::Variable(const Argument& arg) : 
    circuit{arg.getCircuit()}, bits{make_vector([&arg](std::size_t i) {
                return Circuit::Value(*(arg.getInputs().at(i)));
    }, arg.size())}, is_signed{arg.sign()} {}

Variable::Variable(const Variable& var) : 
    circuit{var.getCircuit()}, bits{var.bits}, is_signed{var.sign()} {}

Variable::Variable(Variable&& var) :
    circuit{std::move(var.getCircuit())}, bits{std::move(var.bits)}, is_signed{var.sign()} {}

Variable& Variable::operator=(const Variable& other) {
    if (other.getTypeInfo() == getTypeInfo()) {
        bits = other.bits;
    }
    else {
        bits = std::move(other.cast(getTypeInfo()).bits);
    }
    return *this;
}

Variable& Variable::operator=(Variable&& other) {
    if (other.getTypeInfo() == getTypeInfo()) {
        bits = std::move(other.bits);
    }
    else {
        bits = std::move(other.cast(getTypeInfo()).bits);
    }
    return *this;
}

Variable Variable::generateMask(const Variable& v, TypeInfo info) {
    const auto& b = v.isBit() ? v : v.asBit();
    Variable x(b.getCircuit(), info);
    for (auto& bit : x.bits) {
        bit = b.bits[0];
    }
    return std::move(x);
}

Variable Variable::generateMask(const Variable& v) const {
    return generateMask(v, getTypeInfo());
}

Variable Variable::Not(const Variable& a) {
    Variable x(a.getCircuit(), a.getTypeInfo());
    std::transform(begin(a.bits), end(a.bits), begin(x.bits),
        [](const Circuit::Value& v) {
            return ::Not(v);
        }
    );
    return std::move(x);                
};

Variable Variable::Equal_(const Variable& a, const Variable& b) {
    assert(a.getTypeInfo() == b.getTypeInfo());
    const auto& i = Xnor(a, b);
    return Variable(::MultiAnd(i.bits));
}

Variable Variable::NotEq(const Variable& a, const Variable& b) {
    return !Equal(a, b);
}

Variable Variable::Less_(const Variable& a, const Variable& b) {
    assert(a.getTypeInfo() == b.getTypeInfo());
    Circuit::Value carry_out;
    //subtract *this - t, get the carry
    auto comparison = do_addition(a, ~b, true, &carry_out);
    if (a.sign()) {
        //if signed, have to do a carry computation for the sign bits:
        auto extra_bit = ::Xor(a.bits[a.size()-1], ::Not(b.bits[b.size()-1]));
        return Variable(::Xor(extra_bit, carry_out));
    }
    else {
        //if unsigned, a carry indicates the result of the subtraction is
        //positive or zero, so *this >= t
        return Variable(::Not(carry_out));
    }
}

Variable Variable::LessEq(const Variable& a, const Variable& b) {
    return Less(a, b) | Equal(a, b);
}

Variable Variable::Greater(const Variable& a, const Variable& b) {
    return !LessEq(a, b);
}

Variable Variable::GreaterEq(const Variable& a, const Variable& b) {
    return !Less(a, b);
}

#define DEFINE_BINARY_OP(name) \
    Variable Variable::name##_(const Variable& a, const Variable& b) { \
        assert(a.getTypeInfo() == b.getTypeInfo()); \
        Variable x(a.getCircuit(), a.getTypeInfo()); \
        x.binary_transform(a, b, [](const Circuit::Value& y, const Circuit::Value& z) { \
            return ::name(y, z); \
        }); \
        return std::move(x); \
    }

DEFINE_BINARY_OP(And);
DEFINE_BINARY_OP(Nand);
DEFINE_BINARY_OP(Or);
DEFINE_BINARY_OP(Nor);
DEFINE_BINARY_OP(Xor);
DEFINE_BINARY_OP(Xnor);

#undef DEFINE_BINARY_OP

bool check_multi_types(const std::vector<Variable>& vec) {
    auto a = vec.at(0).getTypeInfo();
    for (const auto& x : vec) {
        if (a != x.getTypeInfo()) return false;
    }
    return true;
}

Variable Variable::MultiAnd(const std::vector<Variable>& vec) {
    assert(check_multi_types(vec));
    Variable x(vec.at(0).getCircuit(), vec.at(0).getTypeInfo());
    x.variadic_transform(vec, ::MultiAnd<std::vector<Circuit::Value>>);
    return std::move(x);
}

Variable Variable::MultiOr(const std::vector<Variable>& vec) {
    assert(check_multi_types(vec));
    Variable x(vec.at(0).getCircuit(), vec.at(0).getTypeInfo());
    x.variadic_transform(vec, ::MultiOr<std::vector<Circuit::Value>>);
    return std::move(x);
}

Variable Variable::Add_(const Variable& a, const Variable& b) {
    assert(a.getTypeInfo() == b.getTypeInfo());
    return do_addition(a, b, false);
}

Variable Variable::Sub_(const Variable& a, const Variable& b) {
    assert(a.getTypeInfo() == b.getTypeInfo());
    return do_addition(a, Not(b), true);
}

Variable Variable::mask_all(const Variable& a, const Variable& b) {
    const Variable& bit = b.isBit() ? b : b.asBit();
    Variable ret(a.getCircuit(), a.getTypeInfo());
    for (unsigned i = 0; i < a.size(); ++i) {
        ret.bits[i] = ::And(a.bits[i], bit.bits[0]);
    }
    return std::move(ret);
}

Variable Variable::do_addition(const Variable& a, const Variable& b, bool c, Circuit::Value* carry_out) {
    auto carry = c ? Circuit::getLiteralTrue(a.getCircuit()) 
        : Circuit::getLiteralFalse(a.getCircuit());
    Variable ret(a.getCircuit(), a.getTypeInfo());
    for (unsigned i = 0; i < a.size(); ++i) {
        std::tie(ret.bits[i], carry) = FullAdder(a.bits[i], b.bits[i], std::move(carry));
    }
    if (carry_out) {
        *carry_out = std::move(carry);
    }
    return std::move(ret);
}

Variable Variable::Shl(const Variable& t, unsigned n) {
    //left shift
    Variable ret(t.getCircuit(), t.getTypeInfo());
    unsigned i = 0;
    for (; i < std::min(n, t.size()); ++i) {
        ret.bits[i] = Circuit::getLiteralFalse(t.getCircuit());
    }
    for (; i < t.size(); ++i) {
        ret.bits[i] = t.bits[i - n];
    }
    return std::move(ret);
}

Variable Variable::Shl_(const Variable& t, const Variable& n) {
    //left shift
    assert(t.getTypeInfo() == n.getTypeInfo());
    std::vector<Variable> integers;
    for (unsigned i = 0; i < t.size(); ++i) {
        auto iseq = (n == i);
        integers.push_back(mask_all(Shl(t, i), iseq));
    }
    return MultiOr(integers);
}

Variable Variable::Shr(const Variable& t, unsigned n) {
    //right shift
    Variable ret(t.getCircuit(), t.getTypeInfo());
    unsigned i = 0;
    for (; i < std::min(n, t.size()); ++i) {
        if (t.sign()) {
            ret.bits[(t.size() - 1) - i] = t.bits[t.size() - 1];
        }
        else {
            ret.bits[(t.size() - 1) - i] = Circuit::getLiteralFalse(t.getCircuit());
        }
    }
    for (; i < t.size(); ++i) {
        ret.bits[(t.size() - 1) - i] = t.bits[(t.size() - 1) - (i - n)];
    }
    return std::move(ret);
}

Variable Variable::Shr_(const Variable& t, const Variable& n) {
    //right shift
    assert(t.getTypeInfo() == n.getTypeInfo());
    std::vector<Variable> integers;
    std::vector<Circuit::Value> bits;
    for (unsigned i = 0; i < t.size(); ++i) {
        auto iseq = (n == i);
        integers.push_back(mask_all(Shr(t, i), iseq));
        bits.push_back(iseq.bits[0]);
    }
    if (t.sign()) {
        //this is Undefined Behavior, but is the most intuitive result
        integers.push_back(mask_all(Variable(-1, t.getCircuit(), t.getTypeInfo()), !Variable(::MultiOr(bits))));
    }
    return MultiOr(integers);
}

Variable Variable::Ternary_(const Variable& t, const Variable& f, const Variable& cond) {
    assert(t.getTypeInfo() == f.getTypeInfo());
    const auto& condbit = cond.isBit() ? cond : cond.asBit();
    return mask_all(t, condbit) | mask_all(f, !condbit);
}

void Variable::DivRem_(const Variable& val, const Variable& div,
        Variable* quot, Variable* rem)
{
    assert(val.getTypeInfo() == div.getTypeInfo());
    //abs is noop for unsigned
    divrem_unsigned(val.abs(), div.abs(), quot, rem);
    if (val.sign()) {
        if (rem) {
            rem->overwrite(Ternary(val.isNeg(), Negative(*rem), *rem));
        }
        if (quot) {
            quot->overwrite(Ternary(val.isNeg() ^ div.isNeg(), Negative(*quot), *quot));
        }
    }
}

void Variable::divrem_unsigned(const Variable& val, const Variable& div,
        Variable* quot, Variable* rem)
{
    Variable q(val.getCircuit(), val.getTypeInfo());
    Variable r(0, val.getCircuit(), val.getTypeInfo());
    //handle division by zero in a somewhat sane manner
    Variable mask = val.generateMask(div == 0);
    for (unsigned i = 0; i < val.size(); ++i) {
        r <<= 1;
        r.bits[0] = val.bits[val.size() - 1 - i];
        auto should_sub = r >= div;
        q.bits[val.size() - 1 - i] = should_sub.bits[0];
        r = Ternary(should_sub, r - div, r);
    }
    if (quot) {
        quot->overwrite(std::move(q));
    }
    if (rem) {
        rem->overwrite(std::move(r));
    }
}

Variable Variable::Mul_full_(const Variable& a, const Variable& b) {
    assert(a.getTypeInfo() == b.getTypeInfo());
    auto ret = mul_unsigned(a.abs(), b.abs());
    if (a.sign()) {
        ret = Variable::Ternary(a.isNeg() ^ b.isNeg(), Variable::Negative(ret), ret);
    }
    return std::move(ret);
}

Variable Variable::mul_unsigned(const Variable& a, const Variable& b) {
    assert(a.getTypeInfo() == b.getTypeInfo());
    auto result_size = a.size()*2;
    auto x = a.cast(TypeInfo(a.sign(), result_size));
    Variable ret(0, a.getCircuit(), x.getTypeInfo());
    for (unsigned i = 0; i < a.size(); ++i) {
        ret = Variable::Ternary(b.bits[i], ret + x, ret);
        x <<= 1;
    }
    return std::move(ret);
}

Variable Variable::cast(TypeInfo info) const {
    Variable ret{this->getCircuit(), info};
    if (info.isBit()) {
        //cast to bit
        ret.bits[0] = ::Not(isZero().bits[0]);
    }
    else if (ret.size() < size()) {
        for (unsigned i = 0; i < ret.size(); ++i) {
            ret.bits[i] = bits[i];
        }
    }
    else {
        unsigned i;
        for (i = 0; i < size(); ++i) {
            ret.bits[i] = bits[i];
        }
        for (; i < ret.size(); ++i) {
            if (ret.sign()) {
                ret.bits[i] = bits[size()-1];
            }
            else {
                ret.bits[i] = Circuit::getLiteralFalse(this->getCircuit());
            }
        }
    }
    return std::move(ret);
}

#define DEFINE_BINARY_OP(op, name) \
    Variable operator op(const Variable& a, const Variable& b) { \
        return Variable::name(a, b); \
    }

DEFINE_BINARY_OP(+, Add);
DEFINE_BINARY_OP(-, Sub);
DEFINE_BINARY_OP(*, Mul);
DEFINE_BINARY_OP(/, Div);
DEFINE_BINARY_OP(%, Rem);
DEFINE_BINARY_OP(<<, Shl);
DEFINE_BINARY_OP(>>, Shr);
DEFINE_BINARY_OP(&, And);
DEFINE_BINARY_OP(&&, LogAnd);
DEFINE_BINARY_OP(|, Or);
DEFINE_BINARY_OP(||, LogOr);
DEFINE_BINARY_OP(^, Xor);
DEFINE_BINARY_OP(==, Equal);
DEFINE_BINARY_OP(!=, NotEq);
DEFINE_BINARY_OP(<, Less);
DEFINE_BINARY_OP(>, Greater);
DEFINE_BINARY_OP(<=, LessEq);
DEFINE_BINARY_OP(>=, GreaterEq);

#undef DEFINE_BINARY_OP

//now we instantiate the various proxy objects
//thank you c++11 so I only have to utter these once...

//Bitwise Operations
decltype(Variable::And) Variable::And;
decltype(Variable::Nand) Variable::Nand;
decltype(Variable::Or) Variable::Or;
decltype(Variable::Nor) Variable::Nor;
decltype(Variable::Xor) Variable::Xor;
decltype(Variable::Xnor) Variable::Xnor;
//Arithmatic Operations
decltype(Variable::Add) Variable::Add;
decltype(Variable::Sub) Variable::Sub;
decltype(Variable::Mul) Variable::Mul;
decltype(Variable::Mul_full) Variable::Mul_full;
decltype(Variable::DivRem) Variable::DivRem;
decltype(Variable::Shl_proxy) Variable::Shl_proxy;
decltype(Variable::Shr_proxy) Variable::Shr_proxy;
//Comparisons
decltype(Variable::Less_proxy) Variable::Less_proxy;
decltype(Variable::Equal_proxy) Variable::Equal_proxy;
//Ternary
decltype(Variable::do_ternary) Variable::do_ternary;

