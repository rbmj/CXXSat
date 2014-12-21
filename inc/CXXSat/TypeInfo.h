#ifndef TYPEINFO_H_INC
#define TYPEINFO_H_INC

#include <type_traits>
#include <ostream>

template <class T>
size_t numbits(T) {
    return sizeof(T)*8;
}

class TypeInfo {
private:
    int info;
    explicit TypeInfo(int i) : info{i} {}
public:
    TypeInfo(const TypeInfo&) = default;
    TypeInfo(bool sign, int size) : info{sign ? -size : size} {}
    TypeInfo& operator=(const TypeInfo&) = default;
    static TypeInfo createBit() {
        return TypeInfo{0};
    }
    template <class Int>
    static TypeInfo create(Int i = Int{}) {
        return TypeInfo{std::is_signed<Int>::value, (int)::numbits(i)};
    }
    bool isBit() const {
        return info == 0;
    }
    int size() const {
        return (info == 0) ? 1 : ((info < 0) ? -info : info);
    }
    bool sign() const {
        return info < 0;
    }
    bool operator==(const TypeInfo& other) const {
        return info == other.info;
    }
    bool operator!=(const TypeInfo& other) const {
        return info != other.info;
    }
};

template<>
inline size_t numbits<TypeInfo>(TypeInfo t) {
    return t.size();
}

std::ostream& operator<<(std::ostream&, const TypeInfo&);

#endif
