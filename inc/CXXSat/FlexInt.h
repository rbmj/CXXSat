#ifndef FLEXINT_H_INC
#define FLEXINT_H_INC

#include <ostream>
#include <type_traits>
#include <algorithm>
#include <stdint.h>
#include <assert.h>

#include <CXXSat/TypeInfo.h>
#include <CXXSat/IntegerTypes.h>

class FlexInt {
private:
    union {
        uint8_t uint8;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;
        int8_t int8;
        int16_t int16;
        int32_t int32;
        int64_t int64;
        
        //rely on polymorphic lambdas for all the hard stuff...
        template <class Op>
        auto do_t(TypeInfo type, Op op) {
            int size = type.size();
            //handle bit or any other nonstandard sizes
            //that we run across
            if (size % 8) {
                size += 8 - (size % 8);
            }
            size = type.sign() ? -size : size;
            switch (size) {
            case 8:
                return op(uint8);
            case 16:
                return op(uint16);
            case 32:
                return op(uint32);
            case 64:
                return op(uint64);
            case -8:
                return op(int8);
            case -16:
                return op(int16);
            case -32:
                return op(int32);
            case -64:
                return op(int64);
            default:
                throw 0; //FIXME
            }
        }
        template <class Op>
        auto do_t(TypeInfo type, Op op) const {
            int size = type.size();
            //handle bit or any other nonstandard sizes
            //that we run across
            if (size % 8) {
                size += 8 - (size % 8);
            }
            size = type.sign() ? -size : size;
            switch (size) {
            case 8:
                return op(uint8);
            case 16:
                return op(uint16);
            case 32:
                return op(uint32);
            case 64:
                return op(uint64);
            case -8:
                return op(int8);
            case -16:
                return op(int16);
            case -32:
                return op(int32);
            case -64:
                return op(int64);
            default:
                throw 0; //FIXME
            }
        }
    } storage;
    TypeInfo type;
    template <class Op>
    auto do_t(Op op) {
        return storage.do_t(type, op);
    }
    template <class Op>
    auto do_t(Op op) const {
        return storage.do_t(type, op);
    }
    //It's Lambda-Ception!
    template <class Op>
    static auto do_binop(const FlexInt& a, const FlexInt& b, Op op) {
        return binop(a, b, [op](const FlexInt& aa, const FlexInt& bb) {
            assert(aa.getTypeInfo() == bb.getTypeInfo());
            return aa.do_t([bb, op](const auto& x) { 
                return bb.do_t([x, op](const auto& y) {
                    return op(x, y);
                });
            });
        });
    }
    template <class Op>
    static auto binop(const FlexInt& a, const FlexInt& b, Op op) {
        //perform the "usual arithmatic conversions"
        auto a_t = a.getTypeInfo();
        auto b_t = b.getTypeInfo();
        unsigned op_size = std::max(a_t.size(), b_t.size());
        bool op_sign;
        if (a_t.sign() == b_t.sign()) {
            op_sign = a_t.sign();
        }
        else {
            //signedness differs
            if (a_t.size() == b_t.size()) {
                op_sign = false; //unsigned prevails if sizes equal
            }
            else {
                op_sign = ((a_t.size() > b_t.size()) ? a_t : b_t).sign();
            }
        }
        //however, all of that said:
        if (op_size < int_size) {
            //all values are converted to int
            op_size = int_size;
            op_sign = true;
        }
        auto info = TypeInfo(op_sign, op_size);
        const auto& new_a = (a_t == info) ? a : a.cast(info);
        const auto& new_b = (b_t == info) ? b : b.cast(info);
        return op(new_a, new_b);
    }
public:
    FlexInt(const FlexInt&) = default;
    template <class Int>
    FlexInt(const Int& i) : FlexInt(i, TypeInfo::create<Int>()) {}
    template <class Int>
    FlexInt(const Int& i, TypeInfo info) : type{info} {
        do_t([i](auto& x) { x = i; });
    }
    FlexInt& operator=(const FlexInt&) = default;
    TypeInfo getTypeInfo() const {
        return type;
    }
    FlexInt cast(TypeInfo info) const {
        int size = info.size();
        //handle bit or any other nonstandard sizes
        //that we run across
        if (size % 8) {
            size += 8 - (size % 8);
        }
        size = info.sign() ? -size : size;
        switch (size) {
        case 8:
            return do_t([](const auto& i) { return FlexInt{(uint8_t)i}; });
        case 16:
            return do_t([](const auto& i) { return FlexInt{(uint16_t)i}; });
        case 32:
            return do_t([](const auto& i) { return FlexInt{(uint32_t)i}; });
        case 64:
            return do_t([](const auto& i) { return FlexInt{(uint64_t)i}; });
        case -8:
            return do_t([](const auto& i) { return FlexInt{(int8_t)i}; });
        case -16:
            return do_t([](const auto& i) { return FlexInt{(int16_t)i}; });
        case -32:
            return do_t([](const auto& i) { return FlexInt{(int32_t)i}; });
        case -64:
            return do_t([](const auto& i) { return FlexInt{(int64_t)i}; });
        default:
            throw 0; //FIXME
        }
    }

    //binary operators
    friend FlexInt operator+(const FlexInt&, const FlexInt&);
    friend FlexInt operator-(const FlexInt&, const FlexInt&);
    friend FlexInt operator*(const FlexInt&, const FlexInt&);
    friend FlexInt operator/(const FlexInt&, const FlexInt&);
    friend FlexInt operator%(const FlexInt&, const FlexInt&);
    friend FlexInt operator>>(const FlexInt&, const FlexInt&);
    friend FlexInt operator<<(const FlexInt&, const FlexInt&);
    friend FlexInt operator&(const FlexInt&, const FlexInt&);
    friend FlexInt operator|(const FlexInt&, const FlexInt&);
    friend FlexInt operator^(const FlexInt&, const FlexInt&);
    friend bool operator<(const FlexInt&, const FlexInt&);
    friend bool operator>(const FlexInt&, const FlexInt&);
    friend bool operator<=(const FlexInt&, const FlexInt&);
    friend bool operator>=(const FlexInt&, const FlexInt&);
    friend bool operator==(const FlexInt&, const FlexInt&);
    friend bool operator!=(const FlexInt&, const FlexInt&);
    friend std::ostream& operator<<(std::ostream&, const FlexInt&);

    //unary operators
    FlexInt operator+() const {
        if (getTypeInfo().size() < int_size) {
            return cast(TypeInfo(true, int_size));
        }
        else return *this;
    }
    FlexInt operator-() const {
        FlexInt x = +(*this);
        FlexInt zero = FlexInt{0, x.getTypeInfo()};
        return zero - x;
    }
    FlexInt operator~() const {
        return do_t([](const auto& x) { return FlexInt{~x}; });
    }
    FlexInt operator!() const {
        return !(bool)*this;
    }
    FlexInt& operator++() {
        do_t([](auto& x) { ++x; });
        return *this;
    }
    FlexInt operator++(int) {
        FlexInt ret{*this};
        ++(*this);
        return ret;
    }
    FlexInt& operator--() {
        do_t([](auto& x) { --x; });
        return *this;
    }
    FlexInt operator--(int) {
        FlexInt ret{*this};
        --(*this);
        return ret;
    }

    //conversion functions
    template <class Int>
    Int as() const {
        Int i;
        do_t([&i](const auto& x) { i = x; });
        return i;
    }
    explicit operator bool() const {
        return do_t([](const auto& x) { return (bool)x; });
    }

    FlexInt& operator+=(const FlexInt& i) {
        *this = *this + i;
        return *this;
    }
    FlexInt& operator-=(const FlexInt& i) {
        *this = *this - i;
        return *this;
    }
    FlexInt& operator*=(const FlexInt& i) {
        *this = *this * i;
        return *this;
    }
    FlexInt& operator/=(const FlexInt& i) {
        *this = *this / i;
        return *this;
    }
    FlexInt& operator%=(const FlexInt& i) {
        *this = *this % i;
        return *this;
    }
    FlexInt& operator>>=(const FlexInt& i) {
        *this = *this >> i;
        return *this;
    }
    FlexInt& operator<<=(const FlexInt& i) {
        *this = *this << i;
        return *this;
    }
    FlexInt& operator&=(const FlexInt& i) {
        *this = *this & i;
        return *this;
    }
    FlexInt& operator|=(const FlexInt& i) {
        *this = *this | i;
        return *this;
    }
    FlexInt& operator^=(const FlexInt& i) {
        *this = *this ^ i;
        return *this;
    }
};


#endif
