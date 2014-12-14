#ifndef SCOPE_H_INC
#define SCOPE_H_INC

#include <CXXSat/DynamicSat.h>
#include <unordered_map>
#include <string>
#include <assert.h>

class Scope {
public:
    Scope(DynCircuit&);
    Scope(Scope&);
    Scope(Scope&, const DynVar&);
    ~Scope();
    DynVar& operator[](const std::string&);
    DynVar* lookup(const std::string&);
    template <class T>
    DynVar& declare(const std::string&);
    template <class T>
    DynVar& declare(const std::string&, T = T{});
    DynVar& declare(const std::string&, bool, unsigned, const char*);
    DynVar& declare(const std::string&, bool, unsigned, long long = 0);
    DynVar& declare(const std::string&, const DynVar&);
private:
    Scope* parent;
    const DynVar* cond;
    DynCircuit& circuit;
    std::unordered_map<std::string, DynVar> variables;
    std::unordered_map<std::string, DynVar> parent_vars;
};

template <class T>
DynVar& Scope::declare(const std::string& s, T val) {
    auto res = variables.insert(std::make_pair(
            s, circuit.getLiteral<T>(val)));
    assert(res.second);
    return res.first->second;
}

#endif
