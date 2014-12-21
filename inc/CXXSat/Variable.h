#ifndef VARIABLE_H_INC
#define VARIABLE_H_INC

#include <CXXSat/Circuit.h>
#include <CXXSat/Range.h>
#include <CXXSat/IntegerTypes.h>
#include <CXXSat/Gates.h>
#include <CXXSat/TypeInfo.h>
#include <CXXSat/FlexInt.h>

#include <array>
#include <memory>
#include <vector>
#include <type_traits>
#include <algorithm>

class Variable;

#define DECLARE_BINARY_OP(op, name) \
    Variable operator op(const Variable&, const Variable&); \
    template <class Int> \
    Variable operator op(const Variable&, const Int&); \
    template <class Int> \
    Variable operator op(const Int&, const Variable&);

DECLARE_BINARY_OP(+, Add);
DECLARE_BINARY_OP(-, Sub);
DECLARE_BINARY_OP(*, Mul);
DECLARE_BINARY_OP(/, Div);
DECLARE_BINARY_OP(%, Rem);
DECLARE_BINARY_OP(<<, Shl);
DECLARE_BINARY_OP(>>, Shr);
DECLARE_BINARY_OP(&, And);
DECLARE_BINARY_OP(&&, LogAnd);
DECLARE_BINARY_OP(|, Or);
DECLARE_BINARY_OP(||, LogOr);
DECLARE_BINARY_OP(^, Xor);
DECLARE_BINARY_OP(==, Equal);
DECLARE_BINARY_OP(!=, NotEq);
DECLARE_BINARY_OP(<, Less);
DECLARE_BINARY_OP(>, Greater);
DECLARE_BINARY_OP(<=, LessEq);
DECLARE_BINARY_OP(>=, GreaterEq);

#undef DECLARE_BINARY_OP

class Variable {
    friend class Circuit;
private:
    std::weak_ptr<Circuit::impl> circuit;
    std::vector<Circuit::Value> bits;
    bool is_signed;
    enum class op_t {
        arith,
        comp,
        logic,
        ternary
    };
    //a functor class that lets us re-use the existing type conversion logic
    template <class Ret, class... ExtraArgs>
    struct binary_operation_generic {
        template <Ret(&Op)(const Variable&, const Variable&, ExtraArgs...), op_t op_type>
        struct binary_operation_t {
            Ret operator()(const Variable& a, const Variable& b, ExtraArgs... args) const;
            binary_operation_t() {}
        };
    };
    template <Variable(&Op)(const Variable&, const Variable&), op_t op_type>
    using binary_operation = binary_operation_generic<Variable>::binary_operation_t<Op, op_type>;
    //Now, declare the underlying methods that do the real computation
    //
    //Precondition for all these: types of operands must be identical
    //Bitwise Operations
    static Variable And_(const Variable&, const Variable&);
    static Variable Nand_(const Variable&, const Variable&);
    static Variable Or_(const Variable&, const Variable&);
    static Variable Nor_(const Variable&, const Variable&);
    static Variable Xor_(const Variable&, const Variable&);
    static Variable Xnor_(const Variable&, const Variable&);
    //Arithmatic Operations
    static Variable Add_(const Variable&, const Variable&);
    static Variable Sub_(const Variable&, const Variable&);
    static Variable Shl_(const Variable&, const Variable&);
    static Variable Shr_(const Variable&, const Variable&);
    static Variable Mul_full_(const Variable&, const Variable&);
    static Variable Mul_(const Variable& a, const Variable& b) {
        return Mul_full(a, b).cast(a.getTypeInfo()); //implicitly truncate
    }
    static void DivRem_(const Variable&, const Variable&, Variable*, Variable*);
    //Comparisons
    static Variable Less_(const Variable&, const Variable&);
    static Variable Equal_(const Variable&, const Variable&);
    //Ternary operator
    static Variable Ternary_(const Variable&, const Variable&, const Variable&);
    static const binary_operation_generic<Variable, const Variable&>::
        binary_operation_t<Ternary_, op_t::ternary> do_ternary;
public:
    const std::weak_ptr<Circuit::impl>& getCircuit() const {
        return circuit;
    }
    unsigned size() const {
        return bits.size();
    }
    bool sign() const {
        return is_signed;
    }
    bool isBit() const {
        return bits.size() == 1;
    }
    TypeInfo getTypeInfo() const {
        if (isBit()) {
            return TypeInfo::createBit();
        }
        else {
            return TypeInfo{sign(), (int)size()};
        }
    }
    Variable(const Argument&);
    Variable(const Variable&);
    Variable(Variable&&);
    explicit Variable(const Circuit::Value& v) : circuit{v.getCircuit()}, bits{v}, is_signed{false} {}
    template <class Int>
    Variable(Int, const std::weak_ptr<Circuit::impl>&, TypeInfo = TypeInfo::create<Int>());
    //provide factory method to allow explicitly passing template param.
    template <class Int>
    static Variable create(Int i, const std::weak_ptr<Circuit::impl>& c) {
        return Variable{i, c};
    }
    static Variable getLiteral(FlexInt i, const std::weak_ptr<Circuit::impl>& c) {
        return i.do_t([c](const auto& x) { return create(x, c); });
    }
    template <class Int>
    Variable getLiteral(Int i) const {
        return create<Int>(i, getCircuit());
    }
    void overwrite(const Variable& v) {
        circuit = v.circuit;
        is_signed = v.is_signed;
        bits = v.bits;
    }
    void overwrite(Variable&& v) {
        circuit = std::move(v.circuit);
        is_signed = v.is_signed;
        bits = std::move(v.bits);
    }
    //Begin operations
    static Variable MultiOr(const std::vector<Variable>&);
    static Variable MultiAnd(const std::vector<Variable>&);
    //Unary operations
    static Variable Negative(const Variable& a) {
        Variable zero(0, a.getCircuit(), a.getTypeInfo());
        return zero - a;
    }
    static Variable Not(const Variable&);
    static Variable Minus(const Variable& a) {
        return Promote(Negative(a));
    }
    static Variable Promote(const Variable& a) {
        if (a.size() < int_size) {
            return a.cast(TypeInfo(true, int_size));
        }
        return a;
    }
    //Bitwise Operations
    static const binary_operation<And_, op_t::logic> And;
    static const binary_operation<Nand_, op_t::logic> Nand;
    static const binary_operation<Or_, op_t::logic> Or;
    static const binary_operation<Nor_, op_t::logic> Nor;
    static const binary_operation<Xor_, op_t::logic> Xor;
    static const binary_operation<Xnor_, op_t::logic> Xnor;
    //Arithmatic Operations
    static const binary_operation<Add_, op_t::arith> Add;
    static const binary_operation<Sub_, op_t::arith> Sub;
    static const binary_operation<Shl_, op_t::arith> Shl_proxy;
    static Variable Shl(const Variable&, unsigned);
    static Variable Shl(const Variable& v, const Variable& i) {
        return Shl_proxy(v, i);
    }
    static const binary_operation<Shr_, op_t::arith> Shr_proxy;
    static Variable Shr(const Variable&, unsigned);
    static Variable Shr(const Variable& v, const Variable& i) {
        return Shr_proxy(v, i);
    }
    static const binary_operation<Mul_full_, op_t::arith> Mul_full;
    static const binary_operation<Mul_, op_t::arith> Mul;
    static const binary_operation_generic<void, Variable*, Variable*>::
        binary_operation_t<DivRem_, op_t::arith> DivRem;
    static Variable Div(const Variable& d, const Variable& v) {
        Variable x(d.getCircuit(), d.getTypeInfo());
        DivRem(d, v, &x, nullptr);
        return std::move(x);
    }
    static Variable Rem(const Variable& d, const Variable& v) {
        Variable x(d.getCircuit(), d.getTypeInfo());
        DivRem(d, v, nullptr, &x);
        return std::move(x);
    }
    static Variable Inc(const Variable& i) {
        return Add(i, i.getLiteral(FlexInt{1, i.getTypeInfo()}));
    }
    static Variable Dec(const Variable& i) {
        return Sub(i, i.getLiteral(FlexInt{1, i.getTypeInfo()}));
    }
    //Comparisons
    static const binary_operation<Less_, op_t::comp> Less_proxy;
    static const binary_operation<Equal_, op_t::comp> Equal_proxy;
    static Variable Less(const Variable& a, const Variable& b) {
        return Less_proxy(a, b);
    }
    static Variable Greater(const Variable&, const Variable&);
    static Variable LessEq(const Variable&, const Variable&);
    static Variable GreaterEq(const Variable&, const Variable&);
    static Variable Equal(const Variable& a, const Variable& b) {
        return Equal_proxy(a, b);
    }
    /* Not worth the trouble...
    template <class Int>
    static Variable Equal(const Variable&, Int);
    template <class Int>
    static Variable NotEq(const Variable& v, Int i) {
        return !Equal(v, i);
    }
    */
    static Variable NotEq(const Variable&, const Variable&);
    //Logical Operations
    static Variable LogAnd(const Variable& a, const Variable& b) {
        return a.asBit() & b.asBit();
    }
    static Variable LogOr(const Variable& a, const Variable& b) {
        return a.asBit() | b.asBit();
    }
    static Variable generateMask(const Variable&, TypeInfo);
    Variable generateMask(const Variable&) const;
    //Ternary Operator
    static Variable Ternary(const Variable& a, const Variable& b, const Variable& c) {
        return do_ternary(b, c, a); //have to change the order so that binary_operator will work as desired
    }
    static Variable Ternary(const Circuit::Value& a, const Variable& b, const Variable& c) {
        return Ternary(Variable(a), b, c); //not optimal, but w/e
    }
    Variable abs() const {
        if (is_signed) {
            return Ternary(bits[size()-1], Negative(*this), *this);
        }
        else {
            return *this;
        }
    }
    Variable& operator=(const Variable&);
    Variable& operator=(Variable&&);
    Variable operator!() const {
        return isZero();
    }
    Variable operator~() const {
        return Not(*this);
    }
    Variable operator-() const {
        return Minus(*this);
    }
    Variable operator+() const {
        return Promote(*this);
    }
    Variable& operator++() {
        return *this = Inc(*this);
    }
    Variable operator++(int) {
        Variable x{*this};
        ++(*this);
        return x;
    }
    Variable& operator--() {
        return *this = Dec(*this);
    }
    Variable operator--(int) {
        Variable x{*this};
        --(*this);
        return x;
    }
    Variable& operator&=(const Variable& v) {
        return *this = *this & v;
    }
    Variable& operator|=(const Variable& v) {
        return *this = *this | v;
    }
    Variable& operator^=(const Variable& v) {
        return *this = *this ^ v;
    }
    Variable& operator+=(const Variable& v) {
        return *this = *this + v;
    }
    Variable& operator-=(const Variable& v) {
        return *this = *this - v;
    }
    Variable& operator<<=(const Variable& v) {
        return *this = *this << v;
    }
    Variable& operator<<=(unsigned i) {
        return *this = *this << i;
    }
    Variable operator<<(unsigned i) const {
        return Shl(*this, i);
    }
    Variable& operator>>=(const Variable& v) {
        return *this = *this >> v;
    }
    Variable& operator>>=(unsigned i) {
        return *this = *this >> i;
    }
    Variable operator>>(unsigned i) const {
        return Shr(*this, i);
    }
    Variable& operator*=(const Variable& v) {
        return *this = *this * v;
    }
    Variable& operator/=(const Variable& v) {
        return *this = *this / v;
    }
    Variable& operator%=(const Variable& v) {
        return *this = *this % v;
    }
    Variable isZero() const {
        if (isBit()) {
            return Variable(::Not(bits[0]));
        }
        return Variable(::MultiOr(bits));
    }
    Variable asBit() const {
        if (isBit()) {
            return *this;
        }
        return !isZero();
    }
    Variable isNeg() const {
        if (sign()) {
            return Variable(bits[size()-1]);
        }
        else {
            return Variable(Circuit::getLiteralFalse(getCircuit()));
        }
    }
    Variable cast(TypeInfo) const;
private:
    static Variable mask_all(const Variable&, const Variable&);
    template <class Op>
    void binary_transform(const Variable&, const Variable&, Op);
    template <class Op>
    void variadic_transform(const std::vector<Variable>&, Op);
    static Variable do_addition(const Variable&, const Variable&, bool, 
            Circuit::Value* = nullptr);
    static void divrem_unsigned(const Variable&, const Variable&, Variable*, Variable*);
    static Variable mul_unsigned(const Variable&, const Variable&);
    Variable(const std::weak_ptr<Circuit::impl>& c, TypeInfo info) :
        circuit{c}, bits{(size_t)info.size()}, is_signed{info.sign()} {}
};

template <class Int>
Variable::Variable(Int t, const std::weak_ptr<Circuit::impl>& c, TypeInfo info) : 
    Variable{c, info}
{
    auto numbits = sizeof(t)*8;
    if (size() < numbits) {
        for (unsigned i = 0; i < size(); ++i) {
            bits[i] = ((t >> i) & 1) ? Circuit::getLiteralTrue(c) : Circuit::getLiteralFalse(c);
        }
    }
    else {
        unsigned i;
        for (i = 0; i < numbits; ++i) {
            bits[i] = ((t >> i) & 1) ? Circuit::getLiteralTrue(c) : Circuit::getLiteralFalse(c);
        }
        for (; i < size(); ++i) {
            if (sign()) {
                bits[i] = ((t >> (numbits - 1)) & 1) ? Circuit::getLiteralTrue(c) : Circuit::getLiteralFalse(c);
            }
            else {
                bits[i] = Circuit::getLiteralFalse(this->getCircuit());
            }
        }
    }
}

//please don't look at this method signature...
template <class Ret, class... ExtraArgs>
template <Ret(&Op)(const Variable&, const Variable&, ExtraArgs...), Variable::op_t op_type>
Ret Variable::binary_operation_generic<Ret, ExtraArgs...>::binary_operation_t<Op, op_type>::
operator()(const Variable& a, const Variable& b, ExtraArgs... args) const {
    if (op_type == op_t::logic && a.isBit() && b.isBit()) {
        //for bitwise operators on single bits, no need to convert
        return Op(a, b, args...);
    }
    if (op_type == op_t::ternary && a.getTypeInfo() == b.getTypeInfo()) {
        //for the ternary operator, follow special rule - no conversion if types identical
        return Op(a, b, args...);
    }
    //otherwise, we have to perform the "usual arithmatic conversions"
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
    if (op_size < int_size) {
        //all values are converted to int
        op_size = int_size;
        op_sign = true;
    }
    auto info = TypeInfo(op_sign, op_size);
    const auto& new_a = (a.getTypeInfo() == info) ? a : a.cast(info);
    const auto& new_b = (b.getTypeInfo() == info) ? b : b.cast(info);
    return Op(new_a, new_b, args...);
}

template <class Op>
void Variable::binary_transform(const Variable& a, const Variable& b, Op op) {
    std::transform(begin(a.bits), end(a.bits), begin(b.bits), begin(bits), op);
}

template <class Op>
void Variable::variadic_transform(const std::vector<Variable>& vec, Op op) {
    unsigned num = vec.size();
    std::vector<Circuit::Value> values(num);
    for (unsigned i = 0; i < size(); ++i) {
        for (unsigned j = 0; j < num; ++j) {
            values[j] = vec[j].bits[i];
        }
        bits[i] = op(values);
    }
}

/* Not worth the trouble...
template <class Int>
Variable Variable::Equal(const Variable& a, Int b) {
    if (sizeof(b)*8 > a.size()) {
        auto shiftout = b >> a.size();
        if (shiftout != 0 || shiftout != (Int)-1) {
            //we're too big - no way we can be equal
            return Variable(Circuit::getLiteralFalse(a.getCircuit()));
        }
    }
    auto numbits = std::min(a.size(), 8*sizeof(b));
    auto bits = std::vector<Circuit::Value>{a.size()};
    unsigned i;
    for (i = 0; i < numbits; ++i) {
        if (!((b >> i) & 1)) {
            bits[i] = ::Not(a.bits[i]);
        }
        else {
            bits[i] = a.bits[i];
        }
    }
    for (; i < a.size(); ++i) {
        if (a.sign()) {
            bits[i] = ::Xnor(a.bits[i], a.bits[numbits-1]);
        }
        else {
            bits[i] = ::Not(a.bits[i]);
        }
    }
    return Variable(::MultiAnd(bits));
}
*/

#define DEFINE_BINARY_OP(op, name) \
    template <class Int> \
    Variable operator op(const Variable& a, const Int& b) { \
        return Variable::name(a, a.getLiteral(b)); \
    } \
    template <class Int> \
    Variable operator op(const Int& a, const Variable& b) { \
        return Variable::name(b.getLiteral(a), b); \
    }

DEFINE_BINARY_OP(+, Add);
DEFINE_BINARY_OP(-, Sub);
DEFINE_BINARY_OP(*, Mul);
DEFINE_BINARY_OP(/, Div);
DEFINE_BINARY_OP(%, Rem);
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

//For << and >> we don't want to clobber the unsigned overload:

template <class Int>
Variable operator <<(const Int& a, const Variable& b) {
    return Variable::Shl(b.getLiteral(a), b);
}
template <class Int>
Variable operator >>(const Int& a, const Variable& b) {
    return Variable::Shr(b.getLiteral(a), b);
}

template <class Int>
Variable Circuit::getLiteral(Int i) const {
    return Variable(i, pimpl_get_self());
}

#endif
