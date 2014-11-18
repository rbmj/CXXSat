#ifndef DYNARG_H_INC
#define DYNARG_H_INC
#include <iostream>
#include <string>
#include <CXXSat/Sat.h>

class Argument;
class DynVar;

class DynArg {
private:
    std::shared_ptr<Argument> arg;
public:
    DynArg(const std::shared_ptr<Argument>& a) : arg(a) {}
    DynVar asValue() const;
    void print(std::ostream&, const Solution&) const;
    std::string toString(const Solution&) const;
};

#endif
