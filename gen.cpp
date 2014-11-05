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

typedef std::function<void(clang::FunctionDecl*)> FindFunctionCallback;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static llvm::cl::extrahelp MoreHelp("\nMore help text...\n");

class FindFunctionVisitor 
        : public clang::RecursiveASTVisitor<FindFunctionVisitor>
{
public:
    explicit FindFunctionVisitor(clang::ASTContext* c) : context(c) {}
    bool VisitFunctionDecl(clang::FunctionDecl* decl) {
        if (decl->getQualifiedNameAsString() == "f") {
            clang::FullSourceLoc l = 
                    context->getFullLoc(decl->getLocStart());
            if (l.isValid()) {
                llvm::outs() << "Found decl at " 
                    << l.getSpellingLineNumber() << ":" 
                    << l.getSpellingColumnNumber() << "\n";
            }
        }
        return true;
    }
private:
    clang::ASTContext* context;
};

class FindFunctionConsumer : public clang::ASTConsumer {
public:
    explicit FindFunctionConsumer(clang::ASTContext* c) : visitor(c) {}
    virtual void HandleTranslationUnit(clang::ASTContext &context) {
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }
private:
    FindFunctionVisitor visitor;
};

class FindFunctionAction : public clang::ASTFrontendAction {
public:
    virtual clang::ASTConsumer* CreateASTConsumer(
            clang::CompilerInstance& compiler,
            llvm::StringRef inFile)
    {
        return new FindFunctionConsumer(&compiler.getASTContext());
    }
};

int main(int argc, const char **argv) {
    if (argc > 2) {
        clang::tooling::runToolOnCode(new FindFunctionAction(argv[2]), argv[1]);
    }
    return 0;
}
