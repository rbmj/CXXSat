#ifndef DYNVAR_H_INC
#define DYNVAR_H_INC

class Variable;

class DynVar {
private:
    std::unique_ptr<Variable> var;
    static std::unique_ptr<Variable> binary_operation(Variable::op_t, Variable*, Variable*);
public:
    DynVar(const Variable&);
    DynVar(std::unique_ptr<Variable>&&);
    DynVar(const DynVar&);
    DynVar(DynVar&&);
    ~DynVar();
    bool isSigned() const;
    unsigned numBits() const;
    bool isBit() const;
    DynVar operator!() const;
    DynVar operator~() const;
    DynVar operator-() const;
    DynVar operator+() const;
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
};

#endif
