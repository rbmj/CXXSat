#ifndef ARGUMENT_H_INC
#define ARGUMENT_H_INC

#include <memory>
#include <vector>
#include <string>
#include <ostream>

#include <CXXSat/Sat.h>
#include <CXXSat/Circuit.h>
#include <CXXSat/IntegerTypes.h>
#include <CXXSat/TypeInfo.h>
#include <CXXSat/FlexInt.h>

typedef std::vector<std::shared_ptr<Circuit::Input>> InputVec;

class Argument {
private:
    bool is_signed;
    InputVec inputs;
    std::weak_ptr<Circuit::impl> circuit;
public:
    const std::weak_ptr<Circuit::impl>& getCircuit() const {
        return circuit;
    }
    InputVec& getInputs() {
        return inputs;
    }
    const InputVec& getInputs() const {
        return inputs;
    }
    size_t size() const {
        return inputs.size();
    }
    bool sign() const {
        return is_signed;
    }
    bool isBit() const {
        return size() == 1;
    }
    TypeInfo getTypeInfo() const {
        if (isBit()) {
            return TypeInfo::createBit();
        }
        else return TypeInfo(sign(), size());
    }
    Argument(const std::weak_ptr<Circuit::impl>& c, TypeInfo info) : is_signed{info.sign()}, circuit(c) {
        std::generate_n(std::back_inserter(inputs), info.size(),
                [&c]() { return Circuit::Input::create(c); });
    }
    Variable asValue() const;
    void print(std::ostream&, const Solution&) const;
    std::string toString(const Solution&) const;
    FlexInt solution(const Solution&) const;
};

template <class Int>
Argument Circuit::addArgument() {
    return addArgument(TypeInfo::create<Int>());
}

#endif
