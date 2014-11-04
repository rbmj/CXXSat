#ifndef VARIABLE_H_INC
#define VARIABLE_H_INC

#include <CXXSat/Circuit.h>
#include <CXXSat/Range.h>
#include <CXXSat/IntegerTypes.h>

#include <array>
#include <memory>
#include <vector>

typedef std::shared_ptr<Circuit::Value> value_ptr;
typedef std::vector<value_ptr> BitVector;
extern template class std::vector<value_ptr>;

class BitVar;

class Variable {
private:
    BitVector bits;
    std::weak_ptr<Circuit::impl> circuit;
    static int class_id;
protected:
    virtual int getTypeID() const = 0;
public:
    template <class Derived>
    class Base;
    template <class T>
    friend class Base;
    Variable(const Variable&) = default;
    Variable(Variable&&) = default;
    template <class... Args>
    Variable(const std::weak_ptr<Circuit::impl>& c, Args&&... args) :
        circuit(c), bits(std::forward<Args>(args)...) {}
    virtual ~Variable() {}
    const std::weak_ptr<Circuit::impl>& getCircuit() const {
        return circuit;
    }
    BitVector& getBits() {
        return bits;
    }
    const BitVector& getBits() const {
        return bits;
    }
    virtual BitVar operator==(const Variable& v) const = 0;
    BitVar operator!=(const Variable& v) const;
};

template <class Derived>
class Variable::Base : public Variable {
private:
    static int getTypeID_() {
        static int id = Variable::class_id++;
        return id;
    }
protected:
    int getTypeID() const {
        return getTypeID_();
    }
public:
    using Variable::Variable;
    std::shared_ptr<Derived> clone_shared() const {
        return std::make_shared<Derived>(*(Derived*)this);
    }
    BitVar operator==(const Variable& v) const;
    virtual BitVar operator==(const Derived& d) const = 0;
};

class BitArgument;
class BitVar : public Variable::Base<BitVar> {
public:
    BitVar(const BitArgument& arg);
    BitVar(const BitVar& v);
    explicit BitVar(value_ptr);
    BitVar(bool, const std::weak_ptr<Circuit::impl>&);
    value_ptr getBit() const;
    static BitVar Not(BitVar);
    static BitVar And(const BitVar&, const BitVar&);
    static BitVar Nand(const BitVar&, const BitVar&);
    static BitVar Or(const BitVar&, const BitVar&);
    static BitVar Nor(const BitVar&, const BitVar&);
    static BitVar Xor(const BitVar&, const BitVar&);
    static BitVar Xnor(const BitVar&, const BitVar&);
    BitVar& operator=(const BitVar& b);
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

template <bool Signed, unsigned N>
class IntVar : public Variable::Base<IntVar<Signed, N>> {
public:
    typedef IntegerType<Signed, N> int_type;
    typedef IntVar<Signed, N> this_t;
    typedef IntArg<Signed, N> arg_t;
    IntVar(const arg_t&);
    IntVar(const this_t&);
    IntVar(int_type, const std::weak_ptr<Circuit::impl>&);
    static this_t Not(const this_t&);
    static this_t And(const this_t&, const this_t&);
    static this_t Nand(const this_t&, const this_t&);
    static this_t Or(const this_t&, const this_t&);
    static this_t Nor(const this_t&, const this_t&);
    static this_t Xor(const this_t&, const this_t&);
    static this_t Xnor(const this_t&, const this_t&);
    static this_t Add(const this_t&, const this_t&);
    static this_t Sub(const this_t&, const this_t&);
    this_t& operator=(const this_t& b);
    this_t operator~() const {
        return Not(*this);
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
    BitVar operator==(const this_t& v) const {
        const auto& i = Xnor(*this, v);
        return BitVar(MultiAnd(i.getBits()));
    }
private:
    template <class... Args>
    explicit IntVar(const std::weak_ptr<Circuit::impl>& c, Args&&... args) 
        : Variable::Base<this_t>(c, std::forward<Args>(args)...) {}
    template <class Op>
    void binary_transform(const this_t&, const this_t&, Op);
    static this_t do_addition(const this_t&, const this_t&, bool carry);
};

#include <CXXSat/Argument.h>

template <bool Signed, unsigned N>
IntVar<Signed, N>::IntVar(const arg_t& arg) : Variable::Base<this_t>(arg.getCircuit()) {
    const auto& i = arg.getInputs();
    auto& b = this->getBits();
    std::transform(begin(i), end(i), std::inserter(b, begin(b)),
            [](const std::shared_ptr<Circuit::Input>& c) {
                return Circuit::Value::create(c);
            });
}

template <bool Signed, unsigned N>
IntVar<Signed, N>::IntVar(const this_t& var) : Variable::Base<this_t>(var.getCircuit()) {
    const auto& b_other = var.getBits();
    auto& b = this->getBits();
    std::transform(begin(b_other), end(b_other), std::inserter(b, begin(b)),
            [](const value_ptr& v) {
                return v->clone();
            });
}

template <bool Signed, unsigned N>
IntVar<Signed, N>::IntVar(int_type t, const std::weak_ptr<Circuit::impl>& c) : Variable::Base<this_t>(c) {
    for (unsigned i = 0; i < N; ++i) {
        this->getBits().push_back((t >> i) & 1 ?
                Circuit::getLiteralTrue(c) :
                Circuit::getLiteralFalse(c));
    }
}

template <bool Signed, unsigned N>
IntVar<Signed, N>& IntVar<Signed, N>::operator=(const this_t& other) {
    //TODO:  Assert circuits equal for all these
    const auto& b_other = other.getBits();
    auto& b = this->getBits();
    b.clear();
    std::transform(begin(b_other), end(b_other), std::inserter(b, begin(b)),
            [](const value_ptr& v) {
                return v->clone();
            });
    return *this;
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Not(const this_t& a) {
    this_t x(a.getCircuit());
    auto& bits = x.getBits();
    const auto& otherbits = a.getBits();
    std::transform(begin(otherbits), end(otherbits), std::inserter(bits, begin(bits)),
        [](const value_ptr& v) {
            return ::Not(v);
        }
    );
    return std::move(x);                
};

template <bool Signed, unsigned N>
template <class Op>
void IntVar<Signed, N>::binary_transform(const this_t& a, const this_t& b, Op op) {
    auto& bits = this->getBits();
    bits.clear();
    const auto& abits = a.getBits();
    const auto& bbits = b.getBits();
    auto inserter = std::inserter(bits, begin(bits));
    std::transform(begin(abits), end(abits), begin(bbits), inserter, op);
}

#define DEFINE_BINARY_OP(name) \
    template <bool Signed, unsigned N> \
    IntVar<Signed, N> IntVar<Signed, N>::name(const this_t& a, const this_t& b) { \
        this_t x(a.getCircuit()); \
        x.binary_transform(a, b, [](const value_ptr& y, const value_ptr& z) { \
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
IntVar<Signed, N> IntVar<Signed, N>::Add(const this_t& a, const this_t& b) {
    return do_addition(a, b, false);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::Sub(const this_t& a, const this_t& b) {
    return do_addition(a, Not(b), true);
}

template <bool Signed, unsigned N>
IntVar<Signed, N> IntVar<Signed, N>::do_addition(const this_t& a, const this_t& b, bool c) {
    auto carry = c ? Circuit::getLiteralTrue(a.getCircuit()) 
        : Circuit::getLiteralFalse(a.getCircuit());
    this_t ret(a.getCircuit(), N);
    auto& x = a.getBits();
    auto& y = b.getBits();
    auto& z = ret.getBits();
    for (unsigned i = 0; i < N; ++i) {
        std::tie(z[i], carry) = FullAdder(x[i], y[i], std::move(carry));
    }
    return ret;
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
