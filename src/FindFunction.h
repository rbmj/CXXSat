// Declares clang::SyntaxOnlyAction.
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>

#include <functional>

typedef std::function<void(clang::FunctionDecl*, clang::ASTContext*)> FindFunctionCallback;

class FindFunctionVisitor 
        : public clang::RecursiveASTVisitor<FindFunctionVisitor>
{
public:
    explicit FindFunctionVisitor(clang::ASTContext* c, const char* f,
            FindFunctionCallback cb) 
        : context(c) , funcname(f), callback(std::move(cb)) {}
    bool VisitFunctionDecl(clang::FunctionDecl* decl) {
        if (decl->getQualifiedNameAsString() == funcname) {
            /*
            clang::FullSourceLoc l = 
                    context->getFullLoc(decl->getLocStart());
            if (l.isValid()) {
                llvm::outs() << "Found decl at " 
                    << l.getSpellingLineNumber() << ":" 
                    << l.getSpellingColumnNumber() << "\n";
            }
            */
            if (decl->doesThisDeclarationHaveABody()) {
                callback(decl, context);
            }
        }
        return true;
    }
private:
    clang::ASTContext* context;
    const char* funcname;
    FindFunctionCallback callback;
};

class FindFunctionConsumer : public clang::ASTConsumer {
public:
    explicit FindFunctionConsumer(clang::ASTContext* c, const char* f,
            FindFunctionCallback cb) : visitor(c, f, std::move(cb)) {}
    virtual void HandleTranslationUnit(clang::ASTContext &context) {
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }
private:
    FindFunctionVisitor visitor;
};

class FindFunctionAction : public clang::ASTFrontendAction {
public:
    FindFunctionAction(const char* fname, FindFunctionCallback cb)
        : func(fname), callback(cb) {}
    virtual clang::ASTConsumer* CreateASTConsumer(
            clang::CompilerInstance& compiler,
            llvm::StringRef/* inFile*/)
    {
        return new FindFunctionConsumer(&compiler.getASTContext(),
                func, callback);
    }
private:
    const char* func;
    FindFunctionCallback callback;
};

class FindFunctionFactory : public clang::tooling::FrontendActionFactory {
public:
    clang::FrontendAction* create() {
        return new FindFunctionAction(fname, cb);
    }
    FindFunctionFactory(const char* f, FindFunctionCallback c) : fname(f), cb(c) {}
private:
    const char* fname;
    FindFunctionCallback cb;
};

