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

void foo(clang::FunctionDecl* decl, clang::ASTContext* con) {
    auto location = con->getFullLoc(decl->getLocStart());
    std::cout << "Found at " << location.getSpellingLineNumber() << '\n';
    std::cout << "Parameters:\n";
    for (auto param : decl->parameters()) {
        auto qualtype = param->getTypeSourceInfo()->getType();
        std::cout << '\t' << param->getNameAsString() << ": ";
        std::cout << qualtype.getAsString() << " (";
        if (qualtype->isIntegerType()) {
            std::cout << "builtin";
            if (qualtype->isUnsignedIntegerType()) {
                std::cout << " unsigned";
            }
            else if (qualtype->isSignedIntegerType()) {
                std::cout << " signed";
            }
        }
        std::cout << ") " << con->getTypeInfo(qualtype).second << '\n';
    }
    /*
    auto* stmt_ = decl->getBody();
    assert(stmt_->getStmtClass() == clang::Stmt::CompoundStmtClass);
    auto* stmt = (clang::CompoundStmt*)stmt_;
    for (auto substmt : stmt->body()) {
        std::cout << "Substatement of type " << substmt->getStmtClassName()
            << '\n';
    }
    */
    printAST(decl->getBody());
}

static llvm::cl::OptionCategory toolCategory("my-tool options");

int main(int argc, const char **argv) {
    if (argc > 1) {
        clang::tooling::CommonOptionsParser opts(--argc, argv + 1, toolCategory);
        auto& compile = opts.getCompilations();
        clang::tooling::ClangTool tool(compile, opts.getSourcePathList());
        FindFunctionFactory factory(argv[1], &foo);
        int result = tool.run(&factory);
    }
    else {
        std::cerr << "USAGE: " << argv[0] << " IDENTIFIER [OPTIONS] -- [CLANG OPTIONS]\n";
    }
    return 0;
}
