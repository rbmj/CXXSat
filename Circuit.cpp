#include "Circuit.h"
#include "Gates.h"

struct Circuit::impl {
    void reg(Input* i) {
        inputs.insert(i);
    }
    void unreg(Input* i) {
        inputs.erase(inputs.find(i));
    }
    void reg(Value* v) {
        outputs.insert(v);
    }
    void unreg(Value* v) {
        outputs.erase(outputs.find(v));
    }
    void reg(Gate* g) {
        gates.insert(g);
    }
    void unreg(Gate* g) {
        gates.erase(gates.find(g));
    }
    void reg(Wire* w) {
        wires.insert(w);
    }
    void unreg(Wire* w) {
        wires.erase(wires.find(w));
    }
    std::unordered_set<Input*> inputs;
    std::unordered_set<Value*> outputs;
    std::unordered_set<Gate*> gates;
    std::unordered_set<Wire*> wires;
    std::vector<std::shared_ptr<Variable>> return_values;
    std::unordered_map<std::string, std::shared_ptr<Argument>> arguments;
    std::shared_ptr<Input> lit0;
    std::shared_ptr<Input> lit1;
    std::weak_ptr<Circuit::impl> self;
};

Circuit::Circuit() {
    pimpl = std::make_shared<Circuit::impl>();
    pimpl->self = pimpl;
}

const std::weak_ptr<Circuit::impl>& Circuit::pimpl_get_self() const {
    return pimpl->self;
}

void Circuit::pimpl_emplace_argument(std::string s,
        std::shared_ptr<Argument> ptr)
{
    pimpl->arguments.emplace(std::move(s), std::move(ptr));
}

void Circuit::yield(const std::shared_ptr<Variable>& v) {
    pimpl->return_values.push_back(v);
}

void Circuit::number() {
    int i = 0;
    for (auto& wire : pimpl->wires) {
        wire->id = ++i;
    }
}

Problem Circuit::generateCNF() const {
    Problem p;
    for (auto& gate : pimpl->gates) {
        gate->emplaceCNF(p);
    }
    return std::move(p);
}

Circuit::Node::Node(const std::weak_ptr<Circuit::impl>& c, Node::NODE_TYPE t)
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

Circuit::Node::~Node() {
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

Circuit::Wire::Wire(const std::shared_ptr<Node>& n) 
    : from(n), c(n->getCircuit()), id(0) 
{
    if (auto circuit = c.lock()) {
        circuit->reg(this);
    }
}

Circuit::Wire::~Wire() {
    if (auto circuit = c.lock()) {
        circuit->unreg(this);
    }
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

std::shared_ptr<Circuit::Value> Nxor(std::shared_ptr<Circuit::Value> a, std::shared_ptr<Circuit::Value> b) {
    return std::make_shared<Circuit::Value>(NxorGate::create(*a, *b));
}

std::shared_ptr<Circuit::Value> Not(std::shared_ptr<Circuit::Value> a) {
    return std::make_shared<Circuit::Value>(NotGate::create(*a));
}

