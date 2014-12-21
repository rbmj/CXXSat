#include "Scope.h"
#include <stdlib.h>
#include <utility>
#include <assert.h>

Scope::Scope(Circuit& c, TypeInfo info) : parent(nullptr), cond(), circuit(c.getPimpl()) {
    declare("_RETURN", info, 0);
}

Scope::Scope(Scope& s) : parent(&s), cond(), circuit(s.circuit) {}
Scope::Scope(Scope& s, const Variable& c) :  parent(&s), cond(std::make_unique<Variable>(c)), circuit(s.circuit) {}

Scope::~Scope() {
    if (!parent) {
        return; //no need to propogate up
    }
    //return all shadowed variables to their parent scope.  If this is a conditional
    //block, then make return of the shadowed variable conditional.
    for (auto var : parent_vars) {
        if (cond) {
            (*parent)[var.first] = Variable::Ternary(*cond, var.second, (*parent)[var.first].asVariable());
        }
        else {
            (*parent)[var.first] = var.second;
        }
    }
}

VarRef Scope::return_value() {
    return (*this)["_RETURN"];
}

Variable* Scope::lookup(const std::string& s) {
    auto it = variables.find(s);
    if (it != variables.end()) {
        return &(it->second);
    }
    //not found in our scope, check shadowed vars
    it = parent_vars.find(s);
    if (it != variables.end()) {
        return &(it->second);
    }
    //not currently shadowed, try parent scope
    auto* var = parent->lookup(s);
    if (var) {
        //found in parent scope, shadow
        auto res = parent_vars.insert(std::make_pair(s, *var));
        assert(res.second);
        return &(res.first->second);
    }
    return nullptr;
}

VarRef Scope::operator[](const std::string& s) {
    auto* var = lookup(s);
    if (!var) {
        //not found in any scope
        throw std::out_of_range("Variable " + s + " not found in scope");
    }
    return VarRef{*this, *var};
}

VarRef Scope::declare(const std::string& s, FlexInt i) {
    return declare(s, getLiteral(i));
}

VarRef Scope::declare(const std::string& s, TypeInfo info, FlexInt i) {
    return declare(s, getLiteral(i.cast(info)));
}

VarRef Scope::declare(const std::string& s, const Variable& val) {
    auto res = variables.insert(std::make_pair(
            s, val));
    assert(res.second);
    return VarRef{*this, res.first->second};
}

void Scope::yield(const VarRef v) {
    (*this)["_RETURN"] = v;
}

