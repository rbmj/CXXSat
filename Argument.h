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

template <bool Signed, unsigned N>
class IntVar;

template <bool Signed, unsigned N>
class IntArg : public Argument {
public:
    IntArg(const std::weak_ptr<Circuit::impl>& c) : Argument() {
        auto& i = getInputs();
        std::generate_n(std::inserter(i, begin(i)), N,
                [&c]() { return Circuit::Input::create(c); });
    }
    std::string sprint() {
        return "IMPLEMENT";
    }
    IntVar<Signed, N> asValue() const;
};

#include "Variable.h"

template <bool Signed, unsigned N>
IntVar<Signed, N> IntArg<Signed, N>::asValue() const {
    return IntVar<Signed, N>(*this);
}

extern template class IntArg<true, 8>;
extern template class IntArg<false, 8>;
extern template class IntArg<true, 16>;
extern template class IntArg<false, 16>;
extern template class IntArg<true, 32>;
extern template class IntArg<false, 32>;
extern template class IntArg<true, 64>;
extern template class IntArg<false, 64>;

typedef IntArg<true, 8> UIntArg8;
typedef IntArg<false, 8> IntArg8;
typedef IntArg<true, 16> UIntArg16;
typedef IntArg<false, 16> IntArg16;
typedef IntArg<true, 32> UIntArg32;
typedef IntArg<false, 32> IntArg32;
typedef IntArg<true, 64> UIntArg64;
typedef IntArg<false, 64> IntArg64;

#endif
