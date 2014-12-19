// Declares clang::SyntaxOnlyAction.
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
// Declares llvm::cl::extrahelp.
#include <llvm/Support/CommandLine.h>
#include <functional>
#include <iostream>

#include "FindFunction.h"
#include "Scope.h"
#include <CXXSat/DynamicSat.h>

void printAST(clang::Stmt* s, unsigned level = 0) {
    if (!s) return;
    for (unsigned i=0; i<level; ++i) {
        std::cout << ' ';
    }
    ++level;
    std::cout << "| " << s->getStmtClassName() << '\n';
    for (auto child : s->children()) {
        printAST(child, level);
    }
}

void parseCompoundStmt(clang::CompoundStmt*, clang::ASTContext*, Scope&);
void parseReturnStmt(clang::ReturnStmt*, clang::ASTContext*, Scope&);
void parseIfStmt(clang::IfStmt*, clang::ASTContext*, Scope&);
void parseDecl(clang::DeclStmt*, clang::ASTContext*, Scope&);
void parseVarDecl(clang::VarDecl*, clang::ASTContext*, Scope&);
DynVar parseExpr(clang::Expr*, clang::ASTContext*, Scope&);
DynVar parseUnaryOperator(clang::UnaryOperator*, clang::ASTContext*, Scope&);
DynVar parseBinaryOperator(clang::BinaryOperator*, clang::ASTContext*, Scope&);
DynVar parseCast(clang::CastExpr*, clang::ASTContext*, Scope&);
DynVar parseDeclRef(clang::DeclRefExpr*, clang::ASTContext*, Scope&);
DynVar parseIntLiteral(clang::IntegerLiteral*, clang::ASTContext*, Scope&);

bool parseStmt(clang::Stmt* stmt, clang::ASTContext* con, Scope& scope) {
    if (!stmt) return true;
    switch (stmt->getStmtClass()) {
    case clang::Stmt::CompoundStmtClass:
        parseCompoundStmt((clang::CompoundStmt*)stmt, con, scope);
        break;
    case clang::Stmt::ReturnStmtClass:
        parseReturnStmt((clang::ReturnStmt*)stmt, con, scope);
        return false;
    case clang::Stmt::NullStmtClass:
        //don't need to do anything
        break;
    case clang::Stmt::IfStmtClass:
        parseIfStmt((clang::IfStmt*)stmt, con, scope);
        break;
    case clang::Stmt::DeclStmtClass:
        parseDecl((clang::DeclStmt*)stmt, con, scope);
        break;
    case clang::Stmt::UnaryOperatorClass:
    case clang::Stmt::BinaryOperatorClass:
    case clang::Stmt::CompoundAssignOperatorClass:
        parseExpr((clang::Expr*)stmt, con, scope);
        break;
    default:
        std::cerr << "Ignoring statement of type " << stmt->getStmtClassName() << '\n';
        break;
    }
    return true;
}

void parseCompoundStmt(clang::CompoundStmt* stmt, clang::ASTContext* con, Scope& s) {
    auto newscope = Scope{s};
    for (auto substmt : stmt->body()) {
        if (!parseStmt(substmt, con, newscope)) break;
    }
}

void parseReturnStmt(clang::ReturnStmt* stmt, clang::ASTContext* con, Scope& scope) {
    scope.yield(parseExpr(stmt->getRetValue(), con, scope));
}

void parseIfStmt(clang::IfStmt* stmt, clang::ASTContext* con, Scope& pscope) {
    auto scope = Scope{pscope};
    auto cvdecl = stmt->getConditionVariable();
    if (cvdecl) {
        parseVarDecl(cvdecl, con, scope);
    }
    auto cond = parseExpr(stmt->getCond(), con, scope).asBool();
    auto then_scope = Scope{scope, cond};
    parseStmt(stmt->getThen(), con, then_scope);
    auto else_scope = Scope{scope, !cond};
    parseStmt(stmt->getElse(), con, else_scope);
}

void parseDecl(clang::DeclStmt* stmt, clang::ASTContext* con, Scope& scope) {
    for (auto decl : stmt->getDeclGroup()) {
        assert(decl->getKind() == clang::Decl::Var);
        parseVarDecl((clang::VarDecl*)decl, con, scope);
    }
}

void parseVarDecl(clang::VarDecl* decl, clang::ASTContext* con, Scope& scope) {
    auto qualtype = decl->getTypeSourceInfo()->getType();
    if (!(qualtype->isIntegerType())) assert(false);
    int size = con->getTypeInfo(qualtype).second;
    bool sign = qualtype->isSignedIntegerType();
    auto var = scope.declare(decl->getNameAsString(), sign, size);
    if (decl->hasInit()) {
        var = parseExpr(decl->getInit(), con, scope);
    }
}

DynVar parseExpr(clang::Expr* expr, clang::ASTContext* con, Scope& scope) {
    switch (expr->getStmtClass()) {
        case clang::Stmt::UnaryOperatorClass:
            return parseUnaryOperator((clang::UnaryOperator*)expr, con, scope);
        case clang::Stmt::CompoundAssignOperatorClass:
        case clang::Stmt::BinaryOperatorClass:
            return parseBinaryOperator((clang::BinaryOperator*)expr, con, scope);
        case clang::Stmt::ImplicitCastExprClass:
        case clang::Stmt::CStyleCastExprClass:
            return parseCast((clang::CastExpr*)expr, con, scope);
        case clang::Stmt::IntegerLiteralClass:
            return parseIntLiteral((clang::IntegerLiteral*)expr, con, scope);
        case clang::Stmt::DeclRefExprClass:
            return parseDeclRef((clang::DeclRefExpr*)expr, con, scope);
        default:
            std::cerr << "Ignoring expression of type " << expr->getStmtClassName() << '\n';
            assert(false);
            throw 0;
    }
}

DynVar parseUnaryOperator(clang::UnaryOperator* expr, clang::ASTContext* con, Scope& scope) {
    auto operand = parseExpr(expr->getSubExpr(), con, scope);
    if (expr->isPostfix()) {
        auto copy = operand.clone();
        if (expr->isIncrementOp()) {
            ++operand;
        }
        else {
            --operand;
        }
        return copy;
    }
    else {
        switch (expr->getOpcode()) {
        case clang::UO_PreInc:
            return ++operand;
        case clang::UO_PreDec:
            return --operand;
        case clang::UO_Plus:
            return +operand;
        case clang::UO_Minus:
            return -operand;
        case clang::UO_Not:
            return ~operand;
        case clang::UO_LNot:
            return !operand;
        default:
            assert(false);
            return operand;
        }
    }
}

DynVar parseBinaryOperator(clang::BinaryOperator* expr, clang::ASTContext* con, Scope& scope) {
    auto lhs = parseExpr(expr->getLHS(), con, scope);
    auto rhs = parseExpr(expr->getRHS(), con, scope);
    switch (expr->getOpcode()) {
    case clang::BO_Mul:
        return lhs * rhs;
    case clang::BO_Div:
        return lhs / rhs;
    case clang::BO_Rem:
        return lhs % rhs;
    case clang::BO_Add:
        return lhs + rhs;
    case clang::BO_Sub:
        return lhs - rhs;
    case clang::BO_Shl:
        return lhs << rhs;
    case clang::BO_Shr:
        return lhs >> rhs;
    case clang::BO_LT:
        return lhs < rhs;
    case clang::BO_GT:
        return lhs > rhs;
    case clang::BO_LE:
        return lhs <= rhs;
    case clang::BO_GE:
        return lhs >= rhs;
    case clang::BO_EQ:
        return lhs == rhs;
    case clang::BO_NE:
        return lhs != rhs;
    case clang::BO_And:
        return lhs & rhs;
    case clang::BO_Or:
        return lhs | rhs;
    case clang::BO_LAnd:
        return lhs && rhs;
    case clang::BO_LOr:
        return lhs || rhs;
    case clang::BO_Xor:
        return lhs ^ rhs;
    case clang::BO_Assign:
        lhs = rhs;
        return lhs;
    case clang::BO_MulAssign:
        lhs *= rhs;
        return lhs;
    case clang::BO_DivAssign:
        lhs /= rhs;
        return lhs;
    case clang::BO_RemAssign:
        lhs %= rhs;
        return lhs;
    case clang::BO_AddAssign:
        lhs += rhs;
        return lhs;
    case clang::BO_SubAssign:
        lhs -= rhs;
        return lhs;
    case clang::BO_ShlAssign:
        lhs <<= rhs;
        return lhs;
    case clang::BO_ShrAssign:
        lhs >>= rhs;
        return lhs;
    case clang::BO_AndAssign:
        lhs &= rhs;
        return lhs;
    case clang::BO_OrAssign:
        lhs |= rhs;
        return lhs;
    case clang::BO_XorAssign:
        lhs ^= rhs;
        return lhs;
    default:
        assert(false);
        return lhs; //can't happen
    }
}

DynVar parseCast(clang::CastExpr* expr, clang::ASTContext* con, Scope& scope) {
    auto type = expr->getType();
    assert(type->isIntegerType());
    return parseExpr(expr->getSubExpr(), con, scope).cast(type->isSignedIntegerType(), con->getTypeInfo(type).second);
}

DynVar parseIntLiteral(clang::IntegerLiteral* expr, clang::ASTContext* con, Scope& scope) {
    auto type = expr->getType();
    assert(type->isIntegerType());
    bool sign = type->isSignedIntegerType();
    unsigned size = con->getTypeInfo(type).second;
    unsigned long long val = expr->getValue().getZExtValue();
    return scope.getLiteral(sign, size, val);
}

DynVar parseDeclRef(clang::DeclRefExpr* expr, clang::ASTContext*, Scope& scope) {
    return scope[expr->getNameInfo().getAsString()];
}

void parseFunc(clang::FunctionDecl* decl, clang::ASTContext* con) {
    auto c = DynCircuit{};
    auto return_type = decl->getReturnType();
    assert(return_type->isIntegerType());
    auto scope = Scope{c, return_type->isSignedIntegerType(), con->getTypeInfo(return_type).second};
    auto args = std::vector<DynArg>{};
    //add all arguments
    for (auto param : decl->parameters()) {
        auto qualtype = param->getTypeSourceInfo()->getType();
        if (qualtype->isIntegerType()) {
            int size = con->getTypeInfo(qualtype).second;
            bool sign = qualtype->isSignedIntegerType();
            if (sign) {
                switch (size) {
                case 8:
                    args.push_back(c.addArgument<int8_t>());
                    break;
                case 16:
                    args.push_back(c.addArgument<int16_t>());
                    break;
                case 32:
                    args.push_back(c.addArgument<int32_t>());
                    break;
                case 64:
                    args.push_back(c.addArgument<int64_t>());
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            else {
                switch (size) {
                case 8:
                    args.push_back(c.addArgument<uint8_t>());
                    break;
                case 16:
                    args.push_back(c.addArgument<uint16_t>());
                    break;
                case 32:
                    args.push_back(c.addArgument<uint32_t>());
                    break;
                case 64:
                    args.push_back(c.addArgument<uint64_t>());
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            scope.declare(param->getNameAsString(), args.back().asValue());
        }
        else {
            assert(false);
        }
    }
    //parse
    parseStmt(decl->getBody(), con, scope);
}

static llvm::cl::OptionCategory toolCategory("my-tool options");

int main(int argc, const char **argv) {
    if (argc > 1) {
        clang::tooling::CommonOptionsParser opts(--argc, argv + 1, toolCategory);
        auto& compile = opts.getCompilations();
        clang::tooling::ClangTool tool(compile, opts.getSourcePathList());
        FindFunctionFactory factory(argv[1], &parseFunc);
        int result = tool.run(&factory);
    }
    else {
        std::cerr << "USAGE: " << argv[0] << " IDENTIFIER [OPTIONS] -- [CLANG OPTIONS]\n";
    }
    return 0;
}
