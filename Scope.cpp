#include "Scope.h"
#include <stdlib.h>
#include <utility>
#include <assert.h>

Scope::Scope(DynCircuit& c, bool sign, unsigned size) : parent(nullptr), cond(), circuit(c) {
    declare("_RETURN", sign, size);
}

Scope::Scope(Scope& s) : parent(&s), cond(), circuit(s.circuit) {}
Scope::Scope(Scope& s, const DynVar& c) :  parent(&s), cond(std::make_unique<DynVar>(c)), circuit(s.circuit) {}

Scope::~Scope() {
    if (!parent) {
        return; //no need to propogate up
    }
    //return all shadowed variables to their parent scope.  If this is a conditional
    //block, then make return of the shadowed variable conditional.
    for (auto var : parent_vars) {
        if (cond) {
            (*parent)[var.first] = DynVar::Ternary(*cond, var.second, (*parent)[var.first]);
        }
        else {
            (*parent)[var.first] = var.second;
        }
    }
}

DynVar* Scope::lookup(const std::string& s) {
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

DynVar& Scope::operator[](const std::string& s) {
    auto* var = lookup(s);
    if (!var) {
        //not found in any scope
        throw std::out_of_range("Variable " + s + " not found in scope");
    }
    return *var;
}

DynVar& Scope::declare(const std::string& s, bool sign, unsigned size, const char* x) {
    return declare(s, sign, size, atoll(x));
}

DynVar& Scope::declare(const std::string& s, bool sign, unsigned size, unsigned long long x) {
    return declare(s, getLiteral(sign, size, x));
}

DynVar Scope::getLiteral(bool sign, unsigned size, unsigned long long x) {
    if (!sign) {
        switch (size) {
        case 8:
            return circuit.getLiteral<uint8_t>((uint8_t)x);
        case 16:
            return circuit.getLiteral<uint16_t>((uint16_t)x);
        case 32:
            return circuit.getLiteral<uint32_t>((uint32_t)x);
        case 64:
            return circuit.getLiteral<uint64_t>((uint64_t)x);
        default:
            throw std::domain_error("Bad number of bits - no such type");
            break;
        }
    }
    else {
        switch (size) {
        case 8:
            return circuit.getLiteral<int8_t>((int8_t)x);
        case 16:
            return circuit.getLiteral<int16_t>((int16_t)x);
        case 32:
            return circuit.getLiteral<int32_t>((int32_t)x);
        case 64:
            return circuit.getLiteral<int64_t>((int64_t)x);
        default:
            throw std::domain_error("Bad number of bits - no such type");
            break;
        }
    }
}

DynVar& Scope::declare(const std::string& s, bool sign, unsigned size, const DynVar& val) {
    auto& x = declare(s, sign, size);
    x = val;
    return x;
}

DynVar& Scope::declare(const std::string& s, const DynVar& val) {
    auto res = variables.insert(std::make_pair(
            s, val));
    assert(res.second);
    return res.first->second;
}

void Scope::yield(const DynVar& v) {
    (*this)["_RETURN"] = v;
}

