#ifndef INTEGER_TYPES_H_INC
#define INTEGER_TYPES_H_INC

#include <type_traits>
#include <stdint.h>

template <unsigned N>
struct Bits {
};
template <>
struct Bits<8> {
    typedef int8_t type;
};
template <>
struct Bits<16> {
    typedef int16_t type;
};
template <>
struct Bits<32> {
    typedef int32_t type;
};
template <>
struct Bits<64> {
    typedef int64_t type;
};

template <bool Signed, unsigned N>
struct IntegerType_t {
    typedef typename Bits<N>::type type;
};

template <unsigned N>
struct IntegerType_t<false, N> {
    typedef typename std::make_unsigned<
        typename Bits<N>::type
    >::type type;
};

template <bool Signed, unsigned N>
using IntegerType = typename IntegerType_t<Signed, N>::type;

#endif
