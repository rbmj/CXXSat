#include "Circuit.h"
#include "Gates.h"

void Circuit::number() {
    int i = 0;
    for (auto& wire : wires) {
        wire->id = ++i;
    }
}

Problem Circuit::generateCNF() const {
    Problem p;
    for (auto& gate : gates) {
        gate->emplaceCNF(p);
    }
    return std::move(p);
}

Node::Node(Circuit* c, Node::NODE_TYPE t) : circuit(c), type(t) {
    switch (type) {
        case NODE_TYPE::INPUT:
            circuit->reg(asInput());
            break;
        case NODE_TYPE::VALUE:
            circuit->reg(asValue());
            break;
        case NODE_TYPE::GATE:
            circuit->reg(asGate());
            break;
        default:
            assert(false);
            break;
    }
}

Node::~Node() {
    switch (type) {
        case NODE_TYPE::INPUT:
            circuit->unreg(asInput());
            break;
        case NODE_TYPE::VALUE:
            circuit->unreg(asValue());
            break;
        case NODE_TYPE::GATE:
            circuit->unreg(asGate());
            break;
        default:
            assert(false);
            break;
    }
}

std::shared_ptr<Value> And(std::shared_ptr<Value> a, std::shared_ptr<Value> b) {
    return std::make_shared<Value>(AndGate::create(*a, *b));
}

std::shared_ptr<Value> Nand(std::shared_ptr<Value> a, std::shared_ptr<Value> b) {
    return std::make_shared<Value>(NandGate::create(*a, *b));
}

std::shared_ptr<Value> Or(std::shared_ptr<Value> a, std::shared_ptr<Value> b) {
    return std::make_shared<Value>(OrGate::create(*a, *b));
}

std::shared_ptr<Value> Nor(std::shared_ptr<Value> a, std::shared_ptr<Value> b) {
    return std::make_shared<Value>(NorGate::create(*a, *b));
}

std::shared_ptr<Value> Xor(std::shared_ptr<Value> a, std::shared_ptr<Value> b) {
    return std::make_shared<Value>(XorGate::create(*a, *b));
}

std::shared_ptr<Value> Nxor(std::shared_ptr<Value> a, std::shared_ptr<Value> b) {
    return std::make_shared<Value>(NxorGate::create(*a, *b));
}

std::shared_ptr<Value> Not(std::shared_ptr<Value> a) {
    return std::make_shared<Value>(NotGate::create(*a));
}

