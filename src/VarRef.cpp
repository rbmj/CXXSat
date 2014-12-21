#include "VarRef.h"
#include "Scope.h"

Variable VarRef::asVariable() const {
    switch (type) {
    case RVALUE:
        return *(storage.rvalue);
    case LVALUE:
        return *(storage.lvalue);
    case INTEGER:
        return scope->getLiteral(storage.integer);
    }
}

