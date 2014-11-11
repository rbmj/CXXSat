#ifndef ARGUMENT_H_INC
#define ARGUMENT_H_INC

#include <memory>
#include <vector>
#include <string>
#include <ostream>

#include <CXXSat/Sat.h>
#include <CXXSat/Circuit.h>
#include <CXXSat/IntegerTypes.h>

typedef std::vector<std::shared_ptr<Circuit::Input>> InputVec;

class Argument {
private:
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
    template <class... Args>
    Argument(const std::weak_ptr<Circuit::impl>& c, Args&&... args) :
        inputs(std::forward<Args>(args)...), circuit(c) {}
    virtual ~Argument() {}
    virtual void print(std::ostream&, const Solution&) const = 0;
    std::string toString(const Solution&) const;
};

class BitVar;
class BitArgument : public Argument {
public:
    BitArgument(const std::weak_ptr<Circuit::impl>& c);
    BitVar asValue() const;
    int getID() const;
    bool solution(const Solution&) const;
    void print(std::ostream&, const Solution&) const;
};

template <bool Signed, unsigned N>
class IntVar;

template <bool Signed, unsigned N>
class IntArg : public Argument {
public:
    typedef IntegerType<Signed, N> int_type;
    IntArg(const std::weak_ptr<Circuit::impl>& c) : Argument(c) {
        auto& i = getInputs();
        std::generate_n(std::inserter(i, begin(i)), N,
                [&c]() { return Circuit::Input::create(c); });
    }
    IntVar<Signed, N> asValue() const;
    int_type solution(const Solution&) const;
    void print(std::ostream&, const Solution&) const;
};

#include <CXXSat/Variable.h>

template <bool Signed, unsigned N>
IntVar<Signed, N> IntArg<Signed, N>::asValue() const {
    return IntVar<Signed, N>(*this);
}

template <bool Signed, unsigned N>
IntegerType<Signed, N> IntArg<Signed, N>::solution(
        const Solution& s) const 
{
    int_type t = 0;
    auto& inputs = getInputs();
    for (unsigned i = 0; i < N; ++i) {
        if (s.at(inputs[i]->getID())) {
            t |= 1 << i;
        }
    }
    return t;
}

template <bool Signed, unsigned N>
void IntArg<Signed, N>::print(std::ostream& o, const Solution& s) const {
    o << +solution(s);
}

extern template class IntArg<true, 8>;
extern template class IntArg<false, 8>;
extern template class IntArg<true, 16>;
extern template class IntArg<false, 16>;
extern template class IntArg<true, 32>;
extern template class IntArg<false, 32>;
extern template class IntArg<true, 64>;
extern template class IntArg<false, 64>;

typedef IntArg<true, 8> IntArg8;
typedef IntArg<false, 8> UIntArg8;
typedef IntArg<true, 16> IntArg16;
typedef IntArg<false, 16> UIntArg16;
typedef IntArg<true, 32> IntArg32;
typedef IntArg<false, 32> UIntArg32;
typedef IntArg<true, 64> IntArg64;
typedef IntArg<false, 64> UIntArg64;

#endif
