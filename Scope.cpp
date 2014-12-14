#include "Scope.h"
#include <stdlib.h>
#include <utility>
#include <assert.h>

Scope::Scope(DynCircuit& c) : circuit(c), parent(nullptr), cond(nullptr) {}
Scope::Scope(Scope& s) : circuit(s.circuit), parent(&s), cond(nullptr) {}
Scope::Scope(Scope& s, const DynVar& c) : circuit(s.circuit), parent(&s), cond(&c) {}

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

DynVar& Scope::declare(const std::string& s, bool sign, unsigned size, long long x) {
    if (!sign) {
        switch (size) {
        case 8:
            return declare<uint8_t>(s, (uint8_t)x);
        case 16:
            return declare<uint16_t>(s, (uint16_t)x);
        case 32:
            return declare<uint32_t>(s, (uint32_t)x);
        case 64:
            return declare<uint64_t>(s, (uint64_t)x);
        default:
            throw std::domain_error("Bad number of bits - no such type");
            break;
        }
    }
    else {
        switch (size) {
        case 8:
            return declare<int8_t>(s, (int8_t)x);
        case 16:
            return declare<int16_t>(s, (int16_t)x);
        case 32:
            return declare<int32_t>(s, (int32_t)x);
        case 64:
            return declare<int64_t>(s, (int64_t)x);
        default:
            throw std::domain_error("Bad number of bits - no such type");
            break;
        }
    }
}
