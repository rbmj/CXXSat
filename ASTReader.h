#ifndef ASTREADER_H_INC
#define ASTREADER_H_INC

#include <clang/AST/Stmt.h>
#include <clang/Ast/Expr.h>
#include <CXXSat/DynamicSat.h>

#include "Scope.h"

class ASTReader {
public:
    void processStmt(clang::Stmt*, Scope&);
    DynVar evalExpr(clang::Expr*, Scope&);
private:
    DynCircuit circuit;
};

#endif
