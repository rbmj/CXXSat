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

Node::Node(const std::weak_ptr<Circuit>& c, Node::NODE_TYPE t)
    : circuit(c), type(t) 
{
    if (auto ptr = circuit.lock()) {
        switch (type) {
            case NODE_TYPE::INPUT:
                ptr->reg(asInput());
                break;
            case NODE_TYPE::VALUE:
                ptr->reg(asValue());
                break;
            case NODE_TYPE::GATE:
                ptr->reg(asGate());
                break;
            default:
                assert(false);
                break;
        }
    }
}

Node::~Node() {
    if (auto ptr = circuit.lock()) {
        switch (type) {
            case NODE_TYPE::INPUT:
                ptr->unreg(asInput());
                break;
            case NODE_TYPE::VALUE:
                ptr->unreg(asValue());
                break;
            case NODE_TYPE::GATE:
                ptr->unreg(asGate());
                break;
            default:
                assert(false);
                break;
        }
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

