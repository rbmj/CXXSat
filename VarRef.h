#ifndef VARREF_H_INC
#define VARREF_H_INC

#include <CXXSat/IntegerTypes.h>
#include <CXXSat/DynVar.h>
#include <utility>
#include "Scope.h"

class VarRef {
    Scope& scope;
    union storage_t {
        std::unique_ptr<DynVar> owned;
        DynVar* unowned;
        struct {
            bool sign;
            unsigned size;
            unsigned long long data;
        } integer;
        storage_t() {}
        storage_t(DynVar& d) : unowned(&d) {}
        storage_t(std::unique_ptr<DynVar>&& d) : owned(std::move(d)) {}
        storage_t(bool sg, unsigned sz, unsigned long long d) : integer{sg, sz, d} {}
        ~storage_t();
    } storage;
    enum {
        OWNED,
        UNOWNED,
        INTEGER
    } type;
    void destroy() {
        if (type == OWNED) {
            storage.owned.~unique_ptr<DynVar>();
        }
    }
    template <class T>
    VarRef& assign_owned(T&& x) {
        if (type == OWNED) {
            *(storage.owned) = std::forward<T>(x);
        }
        else {
            new (&storage.owned) std::unique_ptr<DynVar>(new DynVar(std::forward<T>(x)));
        }
        return *this;
    }
    VarRef(const DynVar& d, Scope& s) : scope{s}, storage{std::make_unique<DynVar>(d)}, type{OWNED} {}
public:
    VarRef(const VarRef& other) : scope{other.scope}, type{other.type} {
        if (type == OWNED) {
            new (&storage.owned) std::unique_ptr<DynVar>(new DynVar(*(other.storage.owned)));
        }
        else if (type == INTEGER) {
            storage.integer = other.storage.integer;
        }
        else if (type == UNOWNED) {
            storage.unowned = other.storage.unowned;
        }
    }
    VarRef(VarRef&& other) : scope{other.scope}, type{other.type} {
        if (type == OWNED) {
            new (&storage.owned) std::unique_ptr<DynVar>(std::move(other.storage.owned));
        }
        else if (type == INTEGER) {
            storage.integer = other.storage.integer;
        }
        else if (type == UNOWNED) {
            storage.unowned = other.storage.unowned;
        }
    }
    VarRef(Scope& s, bool sign, unsigned size, unsigned long long data) : scope{s}, storage{sign, size, data}, type{INTEGER} {}
    VarRef(Scope& s, DynVar& d) : scope{s}, storage{d}, type{UNOWNED} {}
    VarRef create_owned(const DynVar& d) const {
        return create_owned(scope, d);
    }
    static VarRef create_owned(Scope& s, const DynVar& d) {
        return VarRef(d, s);
    }
    ~VarRef() {
        destroy();
    }
    DynVar asDynVar() const {
        switch (type) {
        case OWNED:
            return *(storage.owned);
        case UNOWNED:
            return *(storage.unowned);
        case INTEGER:
            return scope.getLiteral(storage.integer.sign, storage.integer.size, storage.integer.data);
        }
    }
    operator DynVar() const {
        return asDynVar();
    }
    VarRef cast(bool sign, unsigned size) const {
        return create_owned(this->asDynVar().cast(sign, size));
    }
    VarRef operator+() const {
        return create_owned(+(this->asDynVar()));
    }
    VarRef operator-() const {
        return create_owned(-(this->asDynVar()));
    }
    VarRef& operator++() {
        return create_owned(++(this->asDynVar()));
    }
    VarRef& operator--() {
        return create_owned(--(this->asDynVar()));
    }
    VarRef operator++(int) {
        VarRef other{*this};
        create_owned(++(this->asDynVar()));
        return other;
    }
    VarRef operator--(int) {
        VarRef other{*this};
        create_owned(--(this->asDynVar()));
        return other;
    }
};

#endif
