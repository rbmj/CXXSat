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

std::shared_ptr<Circuit::Value> And(std::shared_ptr<Circuit::Value> a, std::shared_ptr<Circuit::Value> b) {
    return std::make_shared<Circuit::Value>(AndGate::create(*a, *b));
}

std::shared_ptr<Circuit::Value> Nand(std::shared_ptr<Circuit::Value> a, std::shared_ptr<Circuit::Value> b) {
    return std::make_shared<Circuit::Value>(NandGate::create(*a, *b));
}

std::shared_ptr<Circuit::Value> Or(std::shared_ptr<Circuit::Value> a, std::shared_ptr<Circuit::Value> b) {
    return std::make_shared<Circuit::Value>(OrGate::create(*a, *b));
}

std::shared_ptr<Circuit::Value> Nor(std::shared_ptr<Circuit::Value> a, std::shared_ptr<Circuit::Value> b) {
    return std::make_shared<Circuit::Value>(NorGate::create(*a, *b));
}

std::shared_ptr<Circuit::Value> Xor(std::shared_ptr<Circuit::Value> a, std::shared_ptr<Circuit::Value> b) {
    return std::make_shared<Circuit::Value>(XorGate::create(*a, *b));
}

std::shared_ptr<Circuit::Value> Xnor(std::shared_ptr<Circuit::Value> a, std::shared_ptr<Circuit::Value> b) {
    return std::make_shared<Circuit::Value>(XnorGate::create(*a, *b));
}

std::shared_ptr<Circuit::Value> Not(std::shared_ptr<Circuit::Value> a) {
    return std::make_shared<Circuit::Value>(NotGate::create(*a));
}

AdderResT FullAdder(
        std::shared_ptr<Circuit::Value> a,
        std::shared_ptr<Circuit::Value> b,
        std::shared_ptr<Circuit::Value> carry)
{
    auto half_sum = Xor(a, b);
    return {
        Xor(half_sum, carry),
        Or(And(a, b), And(half_sum, carry))
    };
}

