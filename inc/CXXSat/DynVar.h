#ifndef DYNVAR_H_INC
#define DYNVAR_H_INC
#include <memory>

class Variable;
class BitVar;

class DynVar {
    friend class BitVar;
private:
    std::unique_ptr<Variable> var;
    using op_t = std::unique_ptr<Variable>(Variable::*)(const Variable&) const;
    static DynVar binary_operation(op_t op, const Variable&, const Variable&);
public:
    DynVar(const Variable&);
    explicit DynVar(std::unique_ptr<Variable>&&);
    DynVar(const DynVar&);
    DynVar(DynVar&&);
    ~DynVar();
    bool isSigned() const;
    unsigned numBits() const;
    bool isBit() const;
    DynVar asBool() const;
    DynVar isZero() const;
    DynVar operator!() const;
    DynVar operator~() const;
    DynVar operator-() const;
    DynVar operator+() const;
    DynVar operator&&(const DynVar&) const;
    DynVar operator||(const DynVar&) const;
    DynVar operator&(const DynVar&) const;
    DynVar operator|(const DynVar&) const;
    DynVar operator^(const DynVar&) const;
    DynVar operator+(const DynVar&) const;
    DynVar operator-(const DynVar&) const;
    DynVar operator*(const DynVar&) const;
    DynVar operator/(const DynVar&) const;
    DynVar operator%(const DynVar&) const;
    DynVar operator<<(const DynVar&) const;
    DynVar operator>>(const DynVar&) const;
    DynVar operator==(const DynVar&) const;
    DynVar operator!=(const DynVar&) const;
    DynVar operator<(const DynVar&) const;
    DynVar operator>(const DynVar&) const;
    DynVar operator<=(const DynVar&) const;
    DynVar operator>=(const DynVar&) const;
    DynVar& operator&=(const DynVar& v) {
        return *this = *this & v;
    }
    DynVar& operator|=(const DynVar& v) {
        return *this = *this | v;
    }
    DynVar& operator^=(const DynVar& v) {
        return *this = *this ^ v;
    }
    DynVar& operator+=(const DynVar& v) {
        return *this = *this + v;
    }
    DynVar& operator-=(const DynVar& v) {
        return *this = *this - v;
    }
    DynVar& operator*=(const DynVar& v) {
        return *this = *this * v;
    }
    DynVar& operator/=(const DynVar& v) {
        return *this = *this / v;
    }
    DynVar& operator%=(const DynVar& v) {
        return *this = *this % v;
    }
    DynVar& operator<<=(const DynVar& v) {
        return *this = *this << v;
    }
    DynVar& operator>>=(const DynVar& v) {
        return *this = *this >> v;
    }
    DynVar operator<<(unsigned) const;
    DynVar operator>>(unsigned) const;
    DynVar& operator<<=(unsigned u) {
        return *this = *this << u;
    }
    DynVar& operator>>=(unsigned u) {
        return *this = *this >> u;
    }
    template <class T>
    DynVar cast() const;
    DynVar& operator=(const DynVar&);
};

#endif
