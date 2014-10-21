#ifndef VARIABLE_H_INC
#define VARIABLE_H_INC

#include "Argument.h"
#include "Range.h"
#include <array>
#include <memory>
#include <vector>

class Value;

typedef std::vector<std::shared_ptr<Value>> BitVector;

class Variable {
private:
    BitVector bits;
public:
    Variable(const Variable&) = default;
    Variable(Variable&&) = default;
    template <class... Args>
    Variable(Args&&... args) : bits(std::forward<Args>(args)...) {}
    virtual ~Variable() = 0;
    BitVector& getBits() {
        return bits;
    }
    const BitVector& getBits() const {
        return bits;
    }
};

class BitVar : public Variable {
private:
    BitVar(std::shared_ptr<Value>);
public:
    BitVar(const BitArgument& arg);
    BitVar(const BitVar& v);
    static BitVar Not(BitVar);
    static BitVar And(const BitVar&, const BitVar&);
    static BitVar Nand(const BitVar&, const BitVar&);
    static BitVar Or(const BitVar&, const BitVar&);
    static BitVar Nor(const BitVar&, const BitVar&);
    static BitVar Xor(const BitVar&, const BitVar&);
    static BitVar Nxor(const BitVar&, const BitVar&);
    BitVar& operator=(const BitVar& b);
    BitVar operator!() const {
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
    int getID() const;
};

#endif
