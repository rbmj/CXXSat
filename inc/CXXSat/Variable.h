#ifndef VARIABLE_H_INC
#define VARIABLE_H_INC

#include <CXXSat/Circuit.h>
#include <CXXSat/Range.h>
#include <CXXSat/IntegerTypes.h>
#include <CXXSat/Gates.h>

#include <array>
#include <memory>
#include <vector>

class BitVar;
class DynVar;

class Variable {
public:
    static constexpr int int_size = 32;
protected:
    typedef std::unique_ptr<Variable> var_ptr;
private:
    std::weak_ptr<Circuit::impl> circuit;
    int type;
    unsigned size() const {
        return (unsigned)(sign() ? -type : type);
    }
    bool sign() const {
        return type < 0;
    }
    bool isBit() const {
        return type == 0;
    }
    using op_t = var_ptr(Variable::*)(const Variable&) const;
    virtual var_ptr do_cast(int) const = 0;
    virtual BitVar isZero() const = 0;
    //begin list of operations:
    virtual var_ptr Neg() const = 0;
    virtual var_ptr Minus() const = 0;
    virtual var_ptr Promote() const = 0;
    virtual var_ptr And(const Variable&) const = 0;
    virtual var_ptr Or(const Variable&) const = 0;
    virtual var_ptr Xor(const Variable&) const = 0;
    virtual var_ptr Add(const Variable&) const = 0;
    virtual var_ptr Sub(const Variable&) const = 0;
    virtual var_ptr Mul(const Variable&) const = 0;
    virtual var_ptr Shr(const Variable&) const = 0;
    virtual var_ptr Shr(unsigned) const = 0;
    virtual var_ptr Shl(const Variable&) const = 0;
    virtual var_ptr Shl(unsigned) const = 0;
    virtual var_ptr Less(const Variable&) const = 0;
    virtual var_ptr Equal(const Variable&) const = 0;
    virtual void DivMod(const Variable&, var_ptr*, var_ptr*) const = 0;
    //end operations
    //pseudo-operations, implemented in terms of actual operaitons
    var_ptr Not() const;
    var_ptr LogAnd(const Variable& v) const;
    var_ptr LogOr(const Variable& v) const;
    var_ptr Greater(const Variable& a) const {
        return GreaterEq(a)->And(*(Equal(a)->Not()));
    }
    var_ptr LessEq(const Variable& a) const {
        return Less(a)->Or(*(Equal(a)));
    }
    var_ptr GreaterEq(const Variable& a) const {
        return Less(a)->Not();
    }
    var_ptr NotEq(const Variable& a) const {
        return Equal(a)->Not();
    }
    var_ptr Div(const Variable& v) const {
        var_ptr ret;
        DivMod(v, &ret, nullptr);
        return std::move(ret);
    }
    var_ptr Mod(const Variable& v) const {
        var_ptr ret;
        DivMod(v, nullptr, &ret);
        return std::move(ret);
    }
protected:
    Variable(const Variable& v) : type(v.type) {}
    virtual std::unique_ptr<Variable> clone() const = 0;
public:
    template <class Derived, int Type>
    class Base;
    template <class Derived, int Type>
    class Base_base;
    template <class Derived, int Type>
    friend class Base;
    template <class Derived, int Type>
    friend class Base_base;
    friend class DynVar;
    static std::unique_ptr<Variable> create(const Variable& v) {
        return v.clone();
    }
    template <class... Args>
    Variable(const std::weak_ptr<Circuit::impl>& c, int t) :
        circuit(c), type(t) {}
    virtual ~Variable() {}
    const std::weak_ptr<Circuit::impl>& getCircuit() const {
        return circuit;
    }
};

//define Type as 0 for a bit, N for uintN_t, -N for intN_t
template <class Derived, int Type>
class Variable::Base_base : public Variable {
    template <class T, int I>
    friend class Variable::Base;
    friend class Variable;
public:
    static constexpr unsigned numBits() {
        return (Type == 0) ? 1 : (unsigned)(
                (Type < 0) ? -Type : Type);
    }
    typedef std::array<Circuit::Value, numBits()> BitArr;
    template <class... Args>
    Base_base(const std::weak_ptr<Circuit::impl>& c, Args&&... args) :
        Variable(c, Type), bits(std::forward<Args>(args)...) {}
    Base_base(const Base_base<Derived, Type>&) = default;
    std::shared_ptr<Derived> clone_shared() const {
        return std::make_shared<Derived>(CAST(*this));
    }
    BitArr& getBits() {
        return bits;
    }
    const BitArr& getBits() const {
        return bits;
    }
private:
    virtual var_ptr And(const Variable& v) const {
        return Derived::And(CAST(*this), CAST(v)).clone();
    }
    virtual var_ptr Or(const Variable& v) const {
        return Derived::Or(CAST(*this), CAST(v)).clone();
    }
    virtual var_ptr Xor(const Variable& v) const {
        return Derived::Xor(CAST(*this), CAST(v)).clone();
    }
    BitArr bits;
    static const Derived& CAST(const Variable& v) {
        assert(v.type == Type);
        return static_cast<const Derived&>(v);
    }
protected:
    std::unique_ptr<Variable> clone() const {
        return std::make_unique<Derived>(CAST(*this));
    }
};

template <class Derived, int Type>
class Variable::Base : public Variable::Base_base<Derived, Type> {
    virtual var_ptr Add(const Variable& v) const {
        return Derived::Add(CAST(*this), CAST(v)).clone();
    }
    virtual var_ptr Sub(const Variable& v) const {
        return Derived::Sub(CAST(*this), CAST(v)).clone();
    }
    virtual var_ptr Mul(const Variable& v) const {
        return Derived::Mul(CAST(*this), CAST(v)).clone();
    }
    virtual var_ptr Shr(unsigned u) const {
        return Derived::Shr(CAST(*this), u).clone();
    }
    virtual var_ptr Shr(const Variable& v) const {
        return Derived::Shr(CAST(*this), CAST(v)).clone();
    }
    virtual var_ptr Shl(unsigned u) const {
        return Derived::Shl(CAST(*this), u).clone();
    }
    virtual var_ptr Shl(const Variable& v) const {
        return Derived::Shl(CAST(*this), CAST(v)).clone();
    }
    virtual var_ptr Neg() const {
        return Derived::Not(CAST(*this)).clone();
    }
    virtual var_ptr Less(const Variable& v) const {
        return Derived::Less(CAST(*this), CAST(v)).clone();
    }
    virtual var_ptr Equal(const Variable& v) const {
        return Derived::Equal(CAST(*this), CAST(v)).clone();
    }
    virtual void DivMod(const Variable& d, var_ptr* qp, var_ptr* rp) const {
        Derived q(this->getCircuit());
        Derived r(this->getCircuit());
        Derived::DivRem(CAST(*this), CAST(d), &q, &r);
        if (qp) *qp = q.clone();
        if (rp) *rp = r.clone();
    }
    virtual var_ptr Minus() const {
        return Derived::Minus(CAST(*this)).clone();
    }
    virtual var_ptr Promote() const {
        return Derived::Promote(CAST(*this)).clone();
    }
    std::unique_ptr<Variable> do_cast(int) const;
public:
    using Base_base<Derived, Type>::Base_base;
    using Base_base<Derived, Type>::CAST;
};

template <>
class Variable::Base<BitVar, 0> : public Variable::Base_base<BitVar, 0> {
    virtual var_ptr Add(const Variable&) const;
    virtual var_ptr Sub(const Variable&) const;
    virtual var_ptr Mul(const Variable&) const;
    virtual var_ptr Shr(const Variable&) const;
    virtual var_ptr Shl(const Variable&) const;
    virtual var_ptr Shr(unsigned) const;
    virtual var_ptr Shl(unsigned) const;
    virtual var_ptr Less(const Variable&) const;
    virtual var_ptr Equal(const Variable&) const;
    virtual var_ptr Neg() const;
    virtual void DivMod(const Variable&, var_ptr*, var_ptr*) const;
    virtual var_ptr Minus() const;
    virtual var_ptr Promote() const;
    std::unique_ptr<Variable> do_cast(int) const;
public:
    using Base_base<BitVar, 0>::Base_base;
    using Base_base<BitVar, 0>::CAST;
};

class BitArgument;
class BitVar : public Variable::Base<BitVar, 0> {
private:
    //compatibility
    BitVar(const std::weak_ptr<Circuit::impl>&);
    typedef Variable::Base<BitVar, 0> Base;
public:
    //for compatibility purposes
    typedef bool int_type;
    BitVar(const BitArgument&);
    BitVar(const BitVar&);
    explicit BitVar(const Circuit::Value&);
    BitVar(bool, const std::weak_ptr<Circuit::impl>&);
    Circuit::Value& getBit();
    const Circuit::Value& getBit() const;
    static BitVar Not(const BitVar&);
    static BitVar And(const BitVar&, const BitVar&);
    static BitVar Nand(const BitVar&, const BitVar&);
    static BitVar Or(const BitVar&, const BitVar&);
    static BitVar Nor(const BitVar&, const BitVar&);
    static BitVar Xor(const BitVar&, const BitVar&);
    static BitVar Xnor(const BitVar&, const BitVar&);
    static BitVar MultiAnd(const std::vector<BitVar>&);
    static BitVar MultiOr(const std::vector<BitVar>&);
    static BitVar FromDynamic(const DynVar&);
    BitVar& operator=(const BitVar& b);
    BitVar isZero() const {
        return Not(*this);
    }
    BitVar operator!() const {
        return Not(*this);
    }
    BitVar operator~() const {
        return Not(*this);
    }
    BitVar& operator&=(const BitVar& v) {
        return *this = *this & v;
    }
    BitVar& operator|=(const BitVar& v) {
        return *this = *this | v;
    }
    BitVar& operator^=(const BitVar& v) {
        return *this = *this ^ v;
    }
    BitVar operator&(const BitVar& v) const {
        return And(*this, v);
    }
    BitVar operator&&(const BitVar& v) const {
        return And(*this, v);
    }
    BitVar operator||(const BitVar& v) const {
        return Or(*this, v);
    }
    BitVar operator|(const BitVar& v) const {
        return Or(*this, v);
    }
    BitVar operator^(const BitVar& v) const {
        return Xor(*this, v);
    }
    BitVar operator==(const BitVar& v) const {
        return Xnor(*this, v);
    }
    BitVar operator!=(const BitVar& v) const {
        return Xor(*this, v);
    }
    int getID() const;
};

template <bool Signed, unsigned N>
class IntArg;

constexpr int getTypeVal(bool Signed, unsigned N) {
    return Signed ? -((int)N) : N;
}

template <bool B, bool Signed, unsigned N>
struct promote_t_ {
    typedef IntVar<true, Variable::int_size> type;
    static constexpr bool sign = true;
    static constexpr unsigned size = Variable::int_size;
    static type promote(const IntVar<Signed, N>& c);
};

template <bool Signed, unsigned N>
struct promote_t_<false, Signed, N> {
    typedef IntVar<Signed, N> type;
    static constexpr bool sign = Signed;
    static constexpr unsigned size = N;
    static type promote(const IntVar<Signed, N>& c);
};

//NOTE:  The bits here are stored little-endian
template <bool Signed, unsigned N>
class IntVar : public Variable::Base<IntVar<Signed, N>, getTypeVal(Signed, N)> {
    template <bool NewSigned, unsigned NewN>
    friend class IntVar;
    template <class T, int I>
    friend class Variable::Base;
private:
    typedef Variable::Base<IntVar<Signed, N>, getTypeVal(Signed, N)> Base;
    constexpr static unsigned multiply_limit = 128;
    static constexpr auto promote = promote_t_<(N < Variable::int_size), Signed, N>::promote;
public:
    static constexpr unsigned numbits = N;
    static constexpr bool issigned = Signed;
    typedef IntegerType<Signed, N> int_type;
    typedef IntVar<Signed, N> this_t;
    typedef IntArg<Signed, N> arg_t;
    typedef typename promote_t_<(N < Variable::int_size), Signed, N>::type promote_t;
    IntVar(const arg_t&);
    IntVar(const this_t&);
    IntVar(const BitVar&);
    IntVar(int_type, const std::weak_ptr<Circuit::impl>&);
    static this_t Negative(const this_t& a) {
        this_t zero(0, a.getCircuit());
        return zero - a;
    }
    static this_t Not(const this_t&);
    static this_t And(const this_t&, const this_t&);
    static this_t Nand(const this_t&, const this_t&);
    static this_t Or(const this_t&, const this_t&);
    static this_t Nor(const this_t&, const this_t&);
    static this_t Xor(const this_t&, const this_t&);
    static this_t Xnor(const this_t&, const this_t&);
    static this_t MultiOr(const std::vector<this_t>&);
    static this_t MultiAnd(const std::vector<this_t>&);
    static this_t Add(const this_t&, const this_t&);
    static this_t Sub(const this_t&, const this_t&);
    static this_t Shl(const this_t&, unsigned);
    static this_t Shl(const this_t&, const this_t&);
    static this_t Shr(const this_t&, unsigned);
    static this_t Shr(const this_t&, const this_t&);
    static BitVar Less(const this_t&, const this_t&);
    static BitVar Greater(const this_t&, const this_t&);
    static BitVar LessEq(const this_t&, const this_t&);
    static BitVar GreaterEq(const this_t&, const this_t&);
    static BitVar Equal(const this_t&, const this_t&);
    static BitVar Equal(const this_t&, int_type);
    static BitVar NotEq(const this_t&, const this_t&);
    static BitVar NotEq(const this_t&, int_type);
    static BitVar LogAnd(const this_t& a, const this_t& b) {
        return !(a.isZero()) & !(b.isZero());
    }
    static BitVar LogOr(const this_t& a, const this_t& b) {
        return !(a.isZero()) | !(b.isZero());
    }
    static promote_t Minus(const this_t& a) {
        return Promote(Negative(a));
    }
    static promote_t Promote(const this_t& a) {
        return promote(a);
    }
    template <unsigned X = N>
    static typename std::enable_if<(X < multiply_limit), IntVar<Signed, N*2>>
    ::type Mul_full(const this_t&, const this_t&);
    template <unsigned X = N>
    static typename std::enable_if<(X >= multiply_limit), IntVar<Signed, N>>
    ::type Mul_full(const this_t& a, const this_t& b) {
        assert(false); //can't do this
        return a;
    }
    static this_t Mul(const this_t& a, const this_t& b) {
        return Mul_full(a, b).template cast<Signed, N>(); //implicitly truncate
    }
    static this_t generateMask(const BitVar&);
    static this_t Ternary(const BitVar&, const this_t&, const this_t&);
    static this_t Ternary(const Circuit::Value& a, const this_t& b, const this_t& c) {
        return Ternary(BitVar(a), b, c); //not optimal, but w/e
    }
    static void DivRem(const this_t&, const this_t&, this_t*, this_t*);
    this_t abs() const {
        if (Signed) {
            return Ternary(this->getBits()[N-1], Negative(*this), *this);
        }
        else {
            return *this;
        }
    }
    this_t& operator=(const this_t& b);
    this_t operator~() const {
        return Not(*this);
    }
    promote_t operator-() const {
        return Minus(*this);
    }
    promote_t operator+() const {
        return Promote(*this);
    }
    this_t& operator&=(const this_t& v) {
        return *this = *this & v;
    }
    this_t& operator|=(const this_t& v) {
        return *this = *this | v;
    }
    this_t& operator^=(const this_t& v) {
        return *this = *this ^ v;
    }
    this_t& operator+=(const this_t& v) {
        return *this = *this + v;
    }
    this_t& operator-=(const this_t& v) {
        return *this = *this - v;
    }
    this_t& operator<<=(const this_t& v) {
        return *this = *this << v;
    }
    this_t& operator<<=(int_type i) {
        return *this = *this << i;
    }
    this_t& operator>>=(const this_t& v) {
        return *this = *this >> v;
    }
    this_t& operator>>=(int_type i) {
        return *this = *this >> i;
    }
    this_t& operator*=(const this_t& v) {
        return *this = *this * v;
    }
    this_t& operator/=(const this_t& v) {
        return *this = *this / v;
    }
    this_t& operator%=(const this_t& v) {
        return *this = *this % v;
    }
    this_t operator&(const this_t& v) const {
        return And(*this, v);
    }
    this_t operator|(const this_t& v) const {
        return Or(*this, v);
    }
    this_t operator^(const this_t& v) const {
        return Xor(*this, v);
    }
    this_t operator+(const this_t& v) const {
        return Add(*this, v);
    }
    this_t operator-(const this_t& v) const {
        return Sub(*this, v);
    }
    this_t operator>>(int_type i) const {
        return Shr(*this, i);
    }
    this_t operator>>(const this_t& v) const {
        return Shr(*this, v);
    }
    this_t operator<<(int_type i) const {
        return Shl(*this, i);
    }
    this_t operator<<(const this_t& v) const {
        return Shl(*this, v);
    }
    this_t operator*(const this_t& v) const {
        return Mul(*this, v);
    }
    this_t operator/(const this_t& v) const {
        this_t x(this->getCircuit());
        DivRem(*this, v, &x, nullptr);
        return std::move(x);
    }
    this_t operator%(const this_t& v) const {
        this_t x(this->getCircuit());
        DivRem(*this, v, nullptr, &x);
        return std::move(x);
    }
    BitVar operator==(const this_t& v) const {
        return Equal(*this, v);
    }
    BitVar operator==(int_type n) const {
        return Equal(*this, n);
    }
    BitVar isZero() const {
        return BitVar(::MultiOr(this->getBits()));
    }
    BitVar operator!=(int_type n) const {
        return NotEq(*this, n);
    }
    BitVar operator!=(const this_t& v) const {
        return NotEq(*this, v);    
    }
    BitVar operator<(const this_t& v) const {
        return Less(*this, v);
    }
    BitVar operator>=(const this_t& v) const {
        return GreaterEq(*this, v);
    }
    BitVar operator<=(const this_t& v) const {
        return LessEq(*this, v);
    }
    BitVar operator>(const this_t& v) const {
        return Greater(*this, v);
    }
    BitVar operator&&(const this_t& a) const {
        return LogAnd(*this, a);
    }
    BitVar operator||(const this_t& a) const {
        return LogOr(*this, a);
    }
    BitVar isNeg() const {
        if (Signed) {
            return BitVar(this->getBits()[N-1]);
        }
        else {
            return BitVar(false, this->getCircuit());
        }
    }
    template <bool NewSigned, unsigned NewN>
    IntVar<NewSigned, NewN> cast() const;
private:
    static this_t mask_all(const this_t&, const BitVar&);
    template <class... Args>
    explicit IntVar(const std::weak_ptr<Circuit::impl>& c, Args&&... args) 
        : Base(c, std::forward<Args>(args)...) {}
    template <class Op>
    void binary_transform(const this_t&, const this_t&, Op);
    template <class Op>
    void variadic_transform(const std::vector<this_t>&, Op);
    static this_t do_addition(const this_t&, const this_t&, bool, 
            Circuit::Value* = nullptr);
    static void divrem_unsigned(const this_t&, const this_t&, this_t*, this_t*);
    template <unsigned X = N>
    static typename std::enable_if<(X < multiply_limit), IntVar<Signed, N*2>>
    ::type mul_unsigned(const this_t&, const this_t&);
    std::unique_ptr<Variable> int_cast(bool, unsigned) const;
};

template <bool B, bool Signed, unsigned N>
typename promote_t_<B, Signed, N>::type promote_t_<B, Signed, N>::promote(const IntVar<Signed, N>& c) {
    return c.template cast<sign, size>();
}

template <bool Signed, unsigned N>
typename promote_t_<false, Signed, N>::type promote_t_<false, Signed, N>::promote(const IntVar<Signed, N>& c) {
    return c;
}

#include <CXXSat/Argument.h>

template <bool Signed, unsigned N>
IntVar<Signed, N>::IntVar(const arg_t& arg) : 
    Base(arg.getCircuit(), make_array<N>([&arg](std::size_t i) {
                return Circuit::Value(*(arg.getInputs().at(i)));
    })) {}

template <bool Signed, unsigned N>
IntVar<Signed, N>::IntVar(const this_t& var) : 
    Base(var.getCircuit(), make_array<N>([&var](std::size_t i) {
                return var.getBits()[i];
    })) {}

template <bool Signed, unsigned N>
IntVar<Signed, N>::IntVar(const BitVar& bit) : 
    Base(bit.getCircuit(), make_array<N>([this, &bit](std::size_t i) {
                return (i == 0) ? bit.getBit() : Circuit::getLiteralFalse(this->getCircuit());
    })) {}

template <bool Signed, unsigned N>
IntVar<Signed, N>::IntVar(int_type t, const std::weak_ptr<Circuit::impl>& c) : 
    Base(c, make_array<N>([t, &c](std::size_t i) {
                return (t >> i) & 1 ? Circuit::getLiteralTrue(c) : Circuit::getLiteralFalse(c);
    })) {}

template <bool Signed, unsigned N>
IntVar<Signed, N>& IntVar<Signed, N>::operator=(const this_t& other) {
    //TODO:  Assert circuits equal for all these
    this->getBits() = other.getBits();
    return *this;
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::generateMask(const BitVar& b) {
    this_t x(b.getCircuit());
    for (auto& bit : x.getBits()) {
        bit = b.getBit();
    }
    return std::move(x);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Not(const this_t& a) {
    this_t x(a.getCircuit());
    auto& bits = x.getBits();
    const auto& otherbits = a.getBits();
    std::transform(begin(otherbits), end(otherbits), begin(bits),
        [](const Circuit::Value& v) {
            return ::Not(v);
        }
    );
    return std::move(x);                
};

template <bool Signed, unsigned N>
BitVar IntVar<Signed, N>::Equal(const this_t& a, const this_t& b) {
    const auto& i = Xnor(a, b);
    return BitVar(::MultiAnd(i.getBits()));
}

template <bool Signed, unsigned N>
BitVar IntVar<Signed, N>::Equal(const this_t& a, int_type b) {
    this_t x(a);
    auto& bits = x.getBits();
    for (unsigned i = 0; i < N; ++i) {
        if (!((b >> i) & 1)) {
            bits[i] = ::Not(bits[i]);
        }
    }
    return BitVar(::MultiAnd(bits));
}

template <bool Signed, unsigned N>
BitVar IntVar<Signed, N>::NotEq(const this_t& a, int_type b) {
    return !Equal(a, b);
}

template <bool Signed, unsigned N>
BitVar IntVar<Signed, N>::NotEq(const this_t& a, const this_t& b) {
    return !Equal(a, b);
}

template <bool Signed, unsigned N>
BitVar IntVar<Signed, N>::Less(const this_t& a, const this_t& b) {
    Circuit::Value carry_out;
    //subtract *this - t, get the carry
    auto comparison = do_addition(a, ~b, true, &carry_out);
    if (Signed) {
        //if signed, have to do a carry computation for the sign bits:
        auto extra_bit = ::Xor(a.getBits()[N-1], ::Not(b.getBits()[N-1]));
        return BitVar(::Xor(extra_bit, carry_out));
    }
    else {
        //if unsigned, a carry indicates the result of the subtraction is
        //positive or zero, so *this >= t
        return BitVar(::Not(carry_out));
    }
}

template <bool Signed, unsigned N>
BitVar IntVar<Signed, N>::LessEq(const this_t& a, const this_t& b) {
    return Less(a, b) | Equal(a, b);
}

template <bool Signed, unsigned N>
BitVar IntVar<Signed, N>::Greater(const this_t& a, const this_t& b) {
    return !LessEq(a, b);
}

template <bool Signed, unsigned N>
BitVar IntVar<Signed, N>::GreaterEq(const this_t& a, const this_t& b) {
    return !Less(a, b);
}

template <bool Signed, unsigned N>
template <class Op>
void IntVar<Signed, N>::binary_transform(const this_t& a, const this_t& b, Op op) {
    auto& bits = this->getBits();
    const auto& abits = a.getBits();
    const auto& bbits = b.getBits();
    std::transform(begin(abits), end(abits), begin(bbits), begin(bits), op);
}

template <bool Signed, unsigned N>
template <class Op>
void IntVar<Signed, N>::variadic_transform(const std::vector<this_t>& vec, Op op) {
    unsigned num = vec.size();
    std::vector<Circuit::Value> values(num);
    auto& bits = this->getBits();
    for (unsigned i = 0; i < N; ++i) {
        for (unsigned j = 0; j < num; ++j) {
            values[j] = vec[j].getBits()[i];
        }
        bits[i] = op(values);
    }
}
    

#define DEFINE_BINARY_OP(name) \
    template <bool Signed, unsigned N> \
    IntVar<Signed, N> IntVar<Signed, N>::name(const this_t& a, const this_t& b) { \
        this_t x(a.getCircuit()); \
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

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::MultiAnd(const std::vector<this_t>& vec) {
    this_t x(vec.at(0).getCircuit());
    x.variadic_transform(vec, ::MultiAnd<std::vector<Circuit::Value>>);
    return std::move(x);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::MultiOr(const std::vector<this_t>& vec) {
    this_t x(vec.at(0).getCircuit());
    x.variadic_transform(vec, ::MultiOr<std::vector<Circuit::Value>>);
    return std::move(x);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Add(const this_t& a, const this_t& b) {
    return do_addition(a, b, false);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Sub(const this_t& a, const this_t& b) {
    return do_addition(a, Not(b), true);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::mask_all(const this_t& a, const BitVar& b) {
    this_t ret(a.getCircuit());
    const auto& x = a.getBits();
    const auto& y = b.getBit();
    auto& z = ret.getBits();
    for (unsigned i = 0; i < N; ++i) {
        z[i] = ::And(x[i], y);
    }
    return std::move(ret);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::do_addition(const this_t& a, const this_t& b, bool c, Circuit::Value* carry_out) {
    auto carry = c ? Circuit::getLiteralTrue(a.getCircuit()) 
        : Circuit::getLiteralFalse(a.getCircuit());
    this_t ret(a.getCircuit());
    auto& x = a.getBits();
    auto& y = b.getBits();
    auto& z = ret.getBits();
    for (unsigned i = 0; i < N; ++i) {
        std::tie(z[i], carry) = FullAdder(x[i], y[i], std::move(carry));
    }
    if (carry_out) {
        *carry_out = std::move(carry);
    }
    return std::move(ret);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Shl(const this_t& t, unsigned n) {
    //left shift
    this_t ret(t.getCircuit());
    auto& t_bits = t.getBits();
    auto& bits = ret.getBits();
    unsigned i = 0;
    for (; i < n; ++i) {
        bits[i] = Circuit::getLiteralFalse(t.getCircuit());
    }
    for (; i < N; ++i) {
        bits[i] = t_bits[i - n];
    }
    return std::move(ret);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Shl(const this_t& t, const this_t& n) {
    //left shift
    std::vector<IntVar<Signed, N>> integers;
    for (int_type i = 0; (unsigned)i < N; ++i) {
        auto iseq = (n == i);
        integers.push_back(mask_all(Shl(t, (unsigned)i), iseq));
    }
    return MultiOr(integers);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Shr(const this_t& t, unsigned n) {
    //right shift
    this_t ret(t.getCircuit());
    auto& t_bits = t.getBits();
    auto& bits = ret.getBits();
    unsigned i = 0;
    for (; i < n; ++i) {
        if (Signed) {
            bits[(N - 1) - i] = t_bits[N - 1];
        }
        else {
            bits[(N - 1) - i] = Circuit::getLiteralFalse(t.getCircuit());
        }
    }
    for (; i < N; ++i) {
        bits[(N - 1) - i] = t_bits[(N - 1) - (i - n)];
    }
    return std::move(ret);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Shr(const this_t& t, const this_t& n) {
    //right shift
    std::vector<IntVar<Signed, N>> integers;
    std::vector<BitVar> bits;
    for (int_type i = 0; (unsigned)i < N; ++i) {
        auto iseq = (n == i);
        integers.push_back(mask_all(Shr(t, (unsigned)i), iseq));
        bits.push_back(iseq);
    }
    if (Signed) {
        //this is Undefined Behavior, but is the most intuitive result
        integers.push_back(mask_all(this_t((int_type)-1, t.getCircuit()), !(BitVar::MultiOr(bits))));
    }
    return MultiOr(integers);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Ternary(const BitVar& b, const this_t& t, const this_t& f) {
    return mask_all(t, b) | mask_all(f, !b);
}

template <bool Signed, unsigned N>
void IntVar<Signed, N>::DivRem(const this_t& val, const this_t& div,
        this_t* quot, this_t* rem)
{
    //abs is noop for unsigned
    divrem_unsigned(val.abs(), div.abs(), quot, rem);
    if (Signed) {
        if (rem) {
            *rem = Ternary(val.isNeg(), Negative(*rem), *rem);
        }
        if (quot) {
            *quot = Ternary(val.isNeg() ^ div.isNeg(), Negative(*quot), *quot);
        }
    }
}

template <bool Signed, unsigned N>
void IntVar<Signed, N>::divrem_unsigned(const this_t& val, const this_t& div,
        this_t* quot, this_t* rem)
{
    this_t q(val.getCircuit());
    this_t r(0, val.getCircuit());
    //handle division by zero in a somewhat sane manner
    this_t mask = generateMask(div == 0);
    for (unsigned i = 0; i < N; ++i) {
        r <<= 1;
        r.getBits()[0] = val.getBits()[N - 1 - i];
        auto should_sub = r >= div;
        q.getBits()[N - 1 - i] = should_sub.getBit();
        r = Ternary(should_sub, r - div, r);
    }
    if (quot) {
        *quot = std::move(q);
    }
    if (rem) {
        *rem = std::move(r);
    }
}

template <bool Signed, unsigned N>
template <unsigned X>
typename std::enable_if<(X < IntVar<Signed, N>::multiply_limit), IntVar<Signed, N*2>>
::type IntVar<Signed, N>::Mul_full(const this_t& a, const this_t& b) {
    auto ret = mul_unsigned(a.abs(), b.abs());
    if (Signed) {
        ret = IntVar<Signed, N*2>::Ternary(a.isNeg() ^ b.isNeg(), IntVar<Signed, N*2>::Negative(ret), ret);
    }
    return std::move(ret);
}

template <bool Signed, unsigned N>
template <unsigned X>
typename std::enable_if<(X < IntVar<Signed, N>::multiply_limit), IntVar<Signed, N*2>>
::type IntVar<Signed, N>::mul_unsigned(const this_t& a, const this_t& b) {
    auto x = a.cast<Signed, N*2>();
    IntVar<Signed, N*2> ret((typename IntVar<Signed, N*2>::int_type)0, a.getCircuit());
    auto& bits = b.getBits();
    for (unsigned i = 0; i < N; ++i) {
        ret = IntVar<Signed, N*2>::Ternary(bits[i], ret + x, ret);
        x <<= (typename IntVar<Signed, N*2>::int_type)1;
    }
    return std::move(ret);
}

template <bool Signed, unsigned N>
template <bool NewSigned, unsigned NewN>
IntVar<NewSigned, NewN> IntVar<Signed, N>::cast() const {
    IntVar<NewSigned, NewN> ret{this->getCircuit()};
    if (NewN < N) {
        for (unsigned i = 0; i < NewN; ++i) {
            ret.getBits()[i] = this->getBits()[i];
        }
    }
    else {
        unsigned i;
        for (i = 0; i < N; ++i) {
            ret.getBits()[i] = this->getBits()[i];
        }
        for (; i < NewN; ++i) {
            if (Signed) {
                ret.getBits()[i] = this->getBits()[N-1];
            }
            else {
                ret.getBits()[i] = Circuit::getLiteralFalse(this->getCircuit());
            }
        }
    }
    return std::move(ret);
}

template <class Derived, int Type>
std::unique_ptr<Variable> Variable::Base<Derived, Type>::do_cast(int newtype) const {
    std::unique_ptr<Variable> ret;
    if (newtype == 0) {
        ret = (!(this->isZero())).clone();
    }
    else {
        return ((Derived*)this)->int_cast(newtype < 0, (unsigned)((newtype < 0) ? -newtype : newtype));
    }
    return ret;
}

template <bool Signed, unsigned N>
std::unique_ptr<Variable> IntVar<Signed, N>::int_cast(bool sign, unsigned size) const {
    if (sign) {
        switch (size) {
        case 8:
            return this->template cast<true, 8>().clone();
        case 16:
            return this->template cast<true, 16>().clone();
        case 32:
            return this->template cast<true, 32>().clone();
        case 64:
            return this->template cast<true, 64>().clone();
        default:
            assert(false);
            break;
        }
    }
    else {
        switch (size) {
        case 8:
            return this->template cast<false, 8>().clone();
        case 16:
            return this->template cast<false, 16>().clone();
        case 32:
            return this->template cast<false, 32>().clone();
        case 64:
            return this->template cast<false, 64>().clone();
        default:
            assert(false);
            break;
        }
    }
    return nullptr; //can't happen
}

extern template class IntVar<true, 8>;
extern template class IntVar<false, 8>;
extern template class IntVar<true, 16>;
extern template class IntVar<false, 16>;
extern template class IntVar<true, 32>;
extern template class IntVar<false, 32>;
extern template class IntVar<true, 64>;
extern template class IntVar<false, 64>;

typedef IntVar<true, 8> IntVar8;
typedef IntVar<false, 8> UIntVar8;
typedef IntVar<true, 16> IntVar16;
typedef IntVar<false, 16> UIntVar16;
typedef IntVar<true, 32> IntVar32;
typedef IntVar<false, 32> UIntVar32;
typedef IntVar<true, 64> IntVar64;
typedef IntVar<false, 64> UIntVar64;


/*
template <class T, std::true_type = true>
struct VarTypeMap_t {};

template <>
struct VarTypeMap_t<bool, true> {
    typedef BitVar type;
};

template <class T>
struct VarTypeMap_t<T, typename std::is_integral<T>::type> {
    typedef typename IntVar<std::is_signed<T>::value, sizeof(T)*8>::type type;
};

template <class T>
using VarTypeMap = typename VarTypeMap_t<T>::type;

*/
#endif
