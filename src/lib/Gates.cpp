#include <CXXSat/Gates.h>

// From http://en.wikipedia.org/wiki/Tseitin_transformation

void NotGate::emplaceCNF(Problem&) {
    //using special wire - no cnf needed
    /*
    auto A = source->ID();
    auto C = getWire()->ID();
    p.addClause({-A, -C});
    p.addClause({A, C});
    */
}

std::shared_ptr<Circuit::Wire> NotGate::create(const Circuit::Value& val) {
    auto gate = std::make_shared<NotGate>(val);
    auto wire = std::make_shared<Circuit::InvertingWire>(gate);
    gate->init(wire);
    return std::move(wire);
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

Circuit::Value And(const Circuit::Value& a, const Circuit::Value& b) {
    return Circuit::Value{AndGate::create(a, b)};
}

Circuit::Value Nand(const Circuit::Value& a, const Circuit::Value& b) {
    return Circuit::Value{NandGate::create(a, b)};
}

Circuit::Value Or(const Circuit::Value& a, const Circuit::Value& b) {
    return Circuit::Value{OrGate::create(a, b)};
}

Circuit::Value Nor(const Circuit::Value& a, const Circuit::Value& b) {
    return Circuit::Value{NorGate::create(a, b)};
}

Circuit::Value Xor(const Circuit::Value& a, const Circuit::Value& b) {
    return Circuit::Value{XorGate::create(a, b)};
}

Circuit::Value Xnor(const Circuit::Value& a, const Circuit::Value& b) {
    return Circuit::Value{XnorGate::create(a, b)};
}

Circuit::Value Not(const Circuit::Value& a) {
    return Circuit::Value{NotGate::create(a)};
}

AdderResT FullAdder(
        const Circuit::Value& a,
        const Circuit::Value& b,
        const Circuit::Value& carry)
{
    auto half_sum = Xor(a, b);
    return {
        {Xor(half_sum, carry)},
        {Or(And(a, b), And(half_sum, carry))}
    };
}

