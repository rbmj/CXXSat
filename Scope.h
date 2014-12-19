#ifndef SCOPE_H_INC
#define SCOPE_H_INC

#include <CXXSat/DynamicSat.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <assert.h>

class Scope {
public:
    Scope(DynCircuit&, bool sign, unsigned size);
    Scope(Scope&);
    Scope(Scope&, const DynVar&);
    Scope(Scope&&) = default;
    ~Scope();
    DynVar& operator[](const std::string&);
    DynVar* lookup(const std::string&);
    DynVar getLiteral(bool, unsigned, unsigned long long = 0);
    template <class T>
    DynVar& declare(const std::string&);
    template <class T>
    DynVar& declare(const std::string&, T = T{});
    DynVar& declare(const std::string&, bool, unsigned, const char*);
    DynVar& declare(const std::string&, bool, unsigned, unsigned long long = 0);
    DynVar& declare(const std::string&, bool, unsigned, const DynVar&);
    DynVar& declare(const std::string&, const DynVar&);
    void yield(const DynVar&);
private:
    Scope* parent;
    std::unique_ptr<DynVar> cond;
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
