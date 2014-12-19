#ifndef INTEGER_TYPES_H_INC
#define INTEGER_TYPES_H_INC

#include <type_traits>
#include <stdint.h>

constexpr int int_size = 32;

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
#if defined __clang__ || defined __GNUC__
template <>
struct Bits<128> {
    typedef __int128_t type;
};
#else
template <>
struct Bits<128> {
    //this may not work
    typedef intmax_t type;
};
#endif

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

#if defined __clang__ || defined __GNUC__
template <>
struct IntegerType_t<false, 128> {
    typedef __uint128_t type;
};
#endif

template <bool Signed, unsigned N>
using IntegerType = typename IntegerType_t<Signed, N>::type;

#endif
