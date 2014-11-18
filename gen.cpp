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

void foo(clang::FunctionDecl* decl, clang::ASTContext* con) {
    auto location = con->getFullLoc(decl->getLocStart());
    std::cout << "Found at " << location.getSpellingLineNumber() << '\n';
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
