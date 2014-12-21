#ifndef SCOPE_H_INC
#define SCOPE_H_INC

#include <CXXSat/Circuit.h>
#include <CXXSat/Variable.h>
#include <CXXSat/FlexInt.h>

#include <unordered_map>
#include <string>
#include <memory>
#include <assert.h>

#include "VarRef.h"

class Scope {
public:
    Scope(Circuit&, TypeInfo info);
    //not a copy constructor, a child constructor!
    explicit Scope(Scope&);
    Scope(Scope&, const Variable&);
    Scope(Scope&&) = default;
    ~Scope();
    VarRef operator[](const std::string&);
    Variable* lookup(const std::string&);
    template <class Int>
    Variable getLiteral(Int i) {
        return Circuit::getLiteral(circuit, i);
    }
    template <class T>
    VarRef declare(const std::string&);
    template <class T>
    VarRef declare(const std::string&, T = T{});
    VarRef declare(const std::string&, FlexInt);
    VarRef declare(const std::string&, TypeInfo, FlexInt);
    VarRef declare(const std::string&, const Variable&);
    void yield(const VarRef);
    VarRef return_value();
private:
    Scope* parent;
    std::unique_ptr<Variable> cond;
    std::weak_ptr<Circuit::impl> circuit;
    std::unordered_map<std::string, Variable> variables;
    std::unordered_map<std::string, Variable> parent_vars;
};

template <class T>
VarRef Scope::declare(const std::string& s, T val) {
    auto res = variables.insert(std::make_pair(
            s, Circuit::getLiteral(circuit, val)));
    assert(res.second);
    return VarRef{*this, res.first->second};
}

#endif
