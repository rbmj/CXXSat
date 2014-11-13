#include <CXXSat/Gates.h>

// From http://en.wikipedia.org/wiki/Tseitin_transformation

void NotGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto C = getWire()->ID();
    p.addClause({-A, -C});
    p.addClause({A, C});
}

void AndGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.addClause({-A, -B, C});
    p.addClause({A, -C});
    p.addClause({B, -C});
}

void NandGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.addClause({-A, -B, -C});
    p.addClause({A, C});
    p.addClause({B, C});
}

void OrGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.addClause({A, B, -C});
    p.addClause({-A, C});
    p.addClause({-B, C});
}

void NorGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.addClause({A, B, C});
    p.addClause({-A, -C});
    p.addClause({-B, -C});
}

void XorGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.addClause({-A, -B, -C});
    p.addClause({A, B, -C});
    p.addClause({A, -B, C});
    p.addClause({-A, B, C});
}

void XnorGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.addClause({-A, -B, C});
    p.addClause({A, B, C});
    p.addClause({A, -B, -C});
    p.addClause({-A, B, -C});
}

void MultiAndGate::emplaceCNF(Problem& p) {
    auto out = getWire()->ID();
    Clause c;
    for (auto& i : inputs) {
        auto x = i->ID();
        p.addClause({x, -out});
        c.push_back(-x);
    }
    c.push_back(out);
    p.addClause(c);
}

void MultiOrGate::emplaceCNF(Problem& p) {
    auto out = getWire()->ID();
    Clause c;
    for (auto& i : inputs) {
        auto x = i->ID();
        p.addClause({-x, out});
        c.push_back(x);
    }
    c.push_back(-out);
    p.addClause(c);
}

Circuit::Value And(Circuit::Value a, Circuit::Value b) {
    return Circuit::Value{AndGate::create(a, b)};
}

Circuit::Value Nand(Circuit::Value a, Circuit::Value b) {
    return Circuit::Value{NandGate::create(a, b)};
}

Circuit::Value Or(Circuit::Value a, Circuit::Value b) {
    return Circuit::Value{OrGate::create(a, b)};
}

Circuit::Value Nor(Circuit::Value a, Circuit::Value b) {
    return Circuit::Value{NorGate::create(a, b)};
}

Circuit::Value Xor(Circuit::Value a, Circuit::Value b) {
    return Circuit::Value{XorGate::create(a, b)};
}

Circuit::Value Xnor(Circuit::Value a, Circuit::Value b) {
    return Circuit::Value{XnorGate::create(a, b)};
}

Circuit::Value Not(Circuit::Value a) {
    return Circuit::Value{NotGate::create(a)};
}

AdderResT FullAdder(
        Circuit::Value a,
        Circuit::Value b,
        Circuit::Value carry)
{
    auto half_sum = Xor(a, b);
    return {
        {Xor(half_sum, carry)},
        {Or(And(a, b), And(half_sum, carry))}
    };
}

