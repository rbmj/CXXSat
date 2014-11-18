#ifndef DYNCIRCUIT_H_INC
#define DYNCIRCUIT_H_INC
#include <CXXSat/IntegerTypes.h>
#include <CXXSat/Sat.h>
#include <CXXSat/DynVar.h>
#include <CXXSat/DynArg.h>
#include <type_traits>

class Circuit;
class BitVar;
class BitArgument;

template <bool, unsigned>
class IntVar;

template <bool, unsigned>
class IntArg;

class DynCircuit {
private:
    struct impl;
    std::unique_ptr<impl> pimpl;
    template <bool Signed, unsigned N>
    struct var_type {
        static constexpr bool sign = Signed;
        static constexpr unsigned size = N;
        typedef IntegerType<Signed, N> type;
        template <bool s = Signed, unsigned n = N>
        struct dep {
            typedef IntVar<s, n> var;
            typedef IntArg<s, n> arg;
        };
    };
    template <class T>
    DynArg addArgumentImpl();
    template <class T>
    DynVar getLiteralImpl(typename T::type) const;
public:
    DynCircuit();
    DynCircuit(const Circuit&);
    DynCircuit(DynCircuit&&);
    ~DynCircuit();
    Problem generateCNF() const;
    Problem generateCNF(const DynVar&) const;
    template <class T>
    DynArg addArgument() {
        return addArgumentImpl<var_type<std::is_signed<T>::value, sizeof(T)*8>>();
    }
    template <class T>
    DynVar getLiteral(T t) const {
        return getLiteralImpl<var_type<std::is_signed<T>::value, sizeof(T)*8>>(t);
    }
};

template <>
struct DynCircuit::var_type<false, 0> {
    static constexpr bool sign = false;
    static constexpr unsigned size = 0;
    typedef bool type;
    template <bool = false, unsigned = 0>
    struct dep {
        typedef BitVar var;
        typedef BitArgument arg;
    };
};

template <>
DynArg DynCircuit::addArgument<bool>();

template <>
DynVar DynCircuit::getLiteral<bool>(bool b) const;

extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 8>>();
extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 16>>();
extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 32>>();
extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 64>>();
extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<true, 8>>();
extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<true, 16>>();
extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<true, 32>>();
extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<true, 64>>();
extern template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 0>>();

extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 8>>(DynCircuit::var_type<false, 8>::type) const;
extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 16>>(DynCircuit::var_type<false, 16>::type) const;
extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 32>>(DynCircuit::var_type<false, 32>::type) const;
extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 64>>(DynCircuit::var_type<false, 64>::type) const;
extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<true, 8>>(DynCircuit::var_type<true, 8>::type) const;
extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<true, 16>>(DynCircuit::var_type<true, 16>::type) const;
extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<true, 32>>(DynCircuit::var_type<true, 32>::type) const;
extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<true, 64>>(DynCircuit::var_type<true, 64>::type) const;
extern template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 0>>(DynCircuit::var_type<false, 0>::type) const;

#endif
