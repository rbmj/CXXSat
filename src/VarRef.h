#ifndef VARREF_H_INC
#define VARREF_H_INC

#include <CXXSat/TypeInfo.h>
#include <CXXSat/FlexInt.h>
#include <CXXSat/Variable.h>
#include <memory>

class Scope;

class VarRef {
    Scope* scope;
    union storage_t {
        std::unique_ptr<Variable> rvalue;
        Variable* lvalue;
        FlexInt integer;
        storage_t() {}
        storage_t(Variable& d) : lvalue(&d) {}
        storage_t(std::unique_ptr<Variable>&& d) : rvalue(std::move(d)) {}
        storage_t(FlexInt x) : integer{x} {}
        ~storage_t() {}
    } storage;
    enum {
        RVALUE,
        LVALUE,
        INTEGER
    } type;
    void destroy() {
        if (type == RVALUE) {
            storage.rvalue.~unique_ptr<Variable>();
        }
    }
    template <class T>
    VarRef& assign_rvalue(T&& x) {
        if (type == RVALUE) {
            *(storage.rvalue) = std::forward<T>(x);
        }
        else {
            new (&storage.rvalue) std::unique_ptr<Variable>(new Variable(std::forward<T>(x)));
        }
        return *this;
    }
    VarRef(const Variable& d, Scope& s) : scope{&s}, storage{std::make_unique<Variable>(d)}, type{RVALUE} {}
    VarRef getLiteral(FlexInt i) const {
        return VarRef{*scope, i};
    }
public:
    VarRef(const VarRef& other) : scope{other.scope}, type{other.type} {
        if (type == RVALUE) {
            new (&storage.rvalue) std::unique_ptr<Variable>(new Variable(*(other.storage.rvalue)));
        }
        else if (type == INTEGER) {
            storage.integer = other.storage.integer;
        }
        else if (type == LVALUE) {
            storage.lvalue = other.storage.lvalue;
        }
    }
    VarRef& operator=(const VarRef& other) {
        if (type == INTEGER || other.type == INTEGER) {
            storage.integer = other.storage.integer;
        }
        else {
            *this = other.asVariable();
        }
        return *this;
    }
    VarRef& operator=(const Variable& other) {
        if (type == INTEGER) {
            type = RVALUE;
            new (&storage.rvalue) std::unique_ptr<Variable>(new Variable(other));
        }
        else if (type == RVALUE) {
            *storage.rvalue = other;
        }
        else {
            *storage.lvalue = other;
        }
        return *this;
    }
    TypeInfo getTypeInfo() const {
        if (type == INTEGER) {
            return storage.integer.getTypeInfo();
        }
        else if (type == RVALUE) {
            return storage.rvalue->getTypeInfo();
        }
        else {
            return storage.lvalue->getTypeInfo();
        }
    }
    VarRef(VarRef&& other) : scope{other.scope}, type{other.type} {
        if (type == RVALUE) {
            new (&storage.rvalue) std::unique_ptr<Variable>(std::move(other.storage.rvalue));
        }
        else if (type == INTEGER) {
            storage.integer = other.storage.integer;
        }
        else if (type == LVALUE) {
            storage.lvalue = other.storage.lvalue;
        }
    }
    VarRef(Scope& s, FlexInt i) : scope{&s}, storage{i}, type{INTEGER} {}
    VarRef(Scope& s, Variable& d) : scope{&s}, storage{d}, type{LVALUE} {}
    VarRef create_rvalue(const Variable& d) const {
        return create_rvalue(*scope, d);
    }
    static VarRef create_rvalue(Scope& s, const Variable& d) {
        return VarRef(d, s);
    }
    ~VarRef() {
        destroy();
    }
    bool isConst() const {
        return type == INTEGER;
    }
    FlexInt asConst() const {
        if (type != INTEGER) throw 0;
        return storage.integer;
    }
    Variable asVariable() const;
    operator Variable() const {
        return asVariable();
    }
    VarRef cast(TypeInfo info) const {
        if (type == INTEGER) return getLiteral(storage.integer.cast(info));
        return create_rvalue(asVariable().cast(info));
    }
    VarRef operator+() const {
        if (type == INTEGER) return getLiteral(+storage.integer);
        return create_rvalue(+(asVariable()));
    }
    VarRef operator-() const {
        if (type == INTEGER) return getLiteral(-storage.integer);
        return create_rvalue(-(asVariable()));
    }
    VarRef& operator++() {
        if (type != LVALUE) throw 0;
        ++(*storage.lvalue);
        return *this;
    }
    VarRef& operator--() {
        if (type != LVALUE) throw 0;
        --(*storage.lvalue);
        return *this;
    }
    VarRef operator++(int) {
        VarRef other{create_rvalue(asVariable())};
        ++(*this);
        return other;
    }
    VarRef operator--(int) {
        VarRef other{create_rvalue(asVariable())};
        --(*this);
        return other;
    }
    VarRef operator!() const {
        if (type == INTEGER) return getLiteral(!storage.integer);
        return create_rvalue(!(asVariable()));
    }
    VarRef operator~() const {
        if (type == INTEGER) return getLiteral(~storage.integer);
        return create_rvalue(~(asVariable()));
    }
#define DEFINE_BINARY_OP(op) \
    VarRef operator op(const VarRef& other) const { \
        if (type == INTEGER && other.type == INTEGER) { \
            return getLiteral(storage.integer op other.storage.integer); \
        } \
        return create_rvalue(asVariable() op other.asVariable()); \
    } \
    VarRef& operator op##=(const VarRef& other) { \
        if (type != LVALUE) throw 0; \
        *storage.lvalue op##= other.asVariable(); \
        return *this; \
    }
    DEFINE_BINARY_OP(+);
    DEFINE_BINARY_OP(-);
    DEFINE_BINARY_OP(*);
    DEFINE_BINARY_OP(/);
    DEFINE_BINARY_OP(%);
    DEFINE_BINARY_OP(&);
    DEFINE_BINARY_OP(|);
    DEFINE_BINARY_OP(^);
#undef DEFINE_BINARY_OP
#define DEFINE_BINARY_OP(op) \
    VarRef operator op(const VarRef& other) const { \
        if (type == INTEGER && other.type == INTEGER) { \
            return getLiteral(storage.integer op other.storage.integer); \
        } \
        return create_rvalue(asVariable() op other.asVariable()); \
    }
    DEFINE_BINARY_OP(&&);
    DEFINE_BINARY_OP(||);
    DEFINE_BINARY_OP(==);
    DEFINE_BINARY_OP(!=);
    DEFINE_BINARY_OP(<);
    DEFINE_BINARY_OP(>);
    DEFINE_BINARY_OP(<=);
    DEFINE_BINARY_OP(>=);
#undef DEFINE_BINARY_OP
    VarRef operator<<(const VarRef& other) const {
        if (type == INTEGER && other.type == INTEGER) {
            return getLiteral(storage.integer << other.storage.integer);
        }
        else if (other.type == INTEGER) {
            return create_rvalue(asVariable() << other.storage.integer.as<unsigned>());
        }
        else return create_rvalue(asVariable() << other.asVariable());
    }
    VarRef& operator<<=(const VarRef& other) {
        if (type != LVALUE) throw 0;
        if (other.type == INTEGER) {
            *storage.lvalue <<= other.storage.integer.as<unsigned>();
        }
        else {
            *storage.lvalue <<= other.asVariable();
        }
        return *this;
    }
    VarRef operator>>(const VarRef& other) const {
        if (type == INTEGER && other.type == INTEGER) {
            return getLiteral(storage.integer >> other.storage.integer);
        }
        else if (other.type == INTEGER) {
            return create_rvalue(asVariable() >> other.storage.integer.as<unsigned>());
        }
        else return create_rvalue(asVariable() >> other.asVariable());
    }
    VarRef& operator>>=(const VarRef& other) {
        if (type != LVALUE) throw 0;
        if (other.type == INTEGER) {
            *storage.lvalue >>= other.storage.integer.as<unsigned>();
        }
        else {
            *storage.lvalue >>= other.asVariable();
        }
        return *this;
    }
};

#endif
