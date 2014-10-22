#ifndef ARGUMENT_H_INC
#define ARGUMENT_H_INC

#include <memory>
#include <vector>
#include <string>

#include "Circuit.h"

typedef std::vector<std::shared_ptr<Circuit::Input>> InputVec;

class Argument {
private:
    InputVec inputs;
public:
    InputVec& getInputs() {
        return inputs;
    }
    const InputVec& getInputs() const {
        return inputs;
    }
    template <class... Args>
    Argument(Args&&... args) : inputs(std::forward<Args>(args)...) {}
    virtual ~Argument() {}
    virtual std::string sprint() = 0;
};

class BitVar;
class BitArgument : public Argument {
public:
    BitArgument(const std::weak_ptr<Circuit::impl>& c);
    std::string sprint() {
        return "IMPLEMENT";
    }
    BitVar asValue() const;
    int getID() const;
};

#endif
