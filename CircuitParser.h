#ifndef CIRCUITPARSER_H_INC
#define CIRCUITPARSER_H_INC

#include "Graph.h"
#include <array>
#include <memory>

class Variable {
public:
    std::unique_ptr<Variable> clone() const = 0;
};

template <unsigned N>
class BitVariable {
public:
    std::unique_ptr<Variable> clone() const {
        for (auto& bit : bits) {
            bit->source()
private:
    std::array<std::unique_ptr<Value*>, N> bits;
};

template <unsigned N, bool Signed>
class Integer : public Variable<N> {
private:
};



#endif
