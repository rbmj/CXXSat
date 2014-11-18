#include <CXXSat/DynCircuit.h>
#include <CXXSat/Circuit.h>
#include <CXXSat/Argument.h>
#include <CXXSat/Variable.h>
#include <CXXSat/DynVar.h>
#include <CXXSat/DynArg.h>
#include "CircuitImpl.h"

struct DynCircuit::impl {
    std::shared_ptr<Circuit::impl> impl;
};

DynCircuit::DynCircuit(const Circuit& c) : pimpl(std::make_unique<impl>()) {
    pimpl->impl = c.pimpl;
}
DynCircuit::DynCircuit() : DynCircuit(Circuit()) {}
DynCircuit::DynCircuit(DynCircuit&&) = default;
DynCircuit::~DynCircuit() {}

template <class T>
DynArg DynCircuit::addArgumentImpl() {
    return DynArg{std::make_shared<typename T::template dep<>::arg>(pimpl->impl)};
}

template <class T>
DynVar DynCircuit::getLiteralImpl(typename T::type t) const {
    return DynVar{typename T::template dep<>::var{t, pimpl->impl}};
}

Problem DynCircuit::generateCNF() const {
    return pimpl->impl->generateCNF();
}

Problem DynCircuit::generateCNF(const DynVar& d) const {
    auto cnf = generateCNF();
    cnf.addClause({BitVar::FromDynamic(d).getBit().getID()});
    return std::move(cnf);
}

template <>
DynArg DynCircuit::addArgument<bool>() {
    return addArgumentImpl<var_type<false, 0>>();
}

template <>
DynVar DynCircuit::getLiteral<bool>(bool b) const {
    return getLiteralImpl<var_type<false, 0>>(b);
}

template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 8>>();
template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 16>>();
template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 32>>();
template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 64>>();
template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<true, 8>>();
template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<true, 16>>();
template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<true, 32>>();
template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<true, 64>>();
template DynArg DynCircuit::addArgumentImpl<DynCircuit::var_type<false, 0>>();

template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 8>>(DynCircuit::var_type<false, 8>::type) const;
template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 16>>(DynCircuit::var_type<false, 16>::type) const;
template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 32>>(DynCircuit::var_type<false, 32>::type) const;
template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 64>>(DynCircuit::var_type<false, 64>::type) const;
template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<true, 8>>(DynCircuit::var_type<true, 8>::type) const;
template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<true, 16>>(DynCircuit::var_type<true, 16>::type) const;
template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<true, 32>>(DynCircuit::var_type<true, 32>::type) const;
template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<true, 64>>(DynCircuit::var_type<true, 64>::type) const;
template DynVar DynCircuit::getLiteralImpl<DynCircuit::var_type<false, 0>>(DynCircuit::var_type<false, 0>::type) const;
