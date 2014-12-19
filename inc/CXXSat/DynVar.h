#ifndef DYNVAR_H_INC
#define DYNVAR_H_INC
#include <memory>
#include <type_traits>

class Variable;
class BitVar;

class DynVar {
    friend class BitVar;
private:
    std::unique_ptr<Variable> var;
    using op_t = std::unique_ptr<Variable>(Variable::*)(const Variable&) const;
    static DynVar binary_operation(op_t op, const Variable&, const Variable&);
    static constexpr int getType(bool sign, unsigned size) {
        return (int)(sign ? -size : size);
    }
    DynVar do_cast(int) const;
    DynVar mask(const BitVar&) const;
public:
    DynVar mask(const DynVar&) const;
    static DynVar Ternary(const DynVar&, const DynVar&, const DynVar&);
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
    DynVar& operator++() {
        return *this += 1;
    }
    DynVar& operator--() {
        return *this -= 1;
    }
    DynVar operator++(int) {
        DynVar other{*this};
        this->operator++();
        return other;
    }
    DynVar operator--(int) {
        DynVar other{*this};
        this->operator--();
        return other;
    }
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
    DynVar cast() const {
        return do_cast(getType(std::is_signed<T>::value, sizeof(T)));
    }
    DynVar cast(bool sign, unsigned size) {
        //will be checked further down
        return do_cast(getType(sign, size));
    }
    DynVar& operator=(const DynVar&);
};

#endif
