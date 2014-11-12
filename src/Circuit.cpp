#include <CXXSat/Circuit.h>
#include <CXXSat/Gates.h>
#include <CXXSat/Variable.h>

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
    std::shared_ptr<Input> lit0;
    std::shared_ptr<Input> lit1;
    std::unordered_set<Input*> inputs;
    std::unordered_set<Value*> outputs;
    std::unordered_set<Gate*> gates;
    std::unordered_set<Wire*> wires;
    std::shared_ptr<Value> final_output;
    std::unordered_set<std::shared_ptr<Argument>> arguments;
    std::weak_ptr<Circuit::impl> self;
};

Circuit::Circuit() {
    pimpl = std::make_shared<Circuit::impl>();
    pimpl->self = pimpl;
    pimpl->lit0 = Input::create(pimpl);
    pimpl->lit1 = Input::create(pimpl);
}

const std::weak_ptr<Circuit::impl>& Circuit::pimpl_get_self() const {
    return pimpl->self;
}

void Circuit::pimpl_emplace_argument(std::shared_ptr<Argument> ptr) {
    pimpl->arguments.emplace(std::move(ptr));
}

Problem Circuit::generateCNF(BitVar b) const {
    auto cnf = generateCNF();
    cnf.addClause({b.getBit()->getID()});
    return std::move(cnf);
}

void Circuit::number() const {
    int i = 0;
    for (auto& wire : pimpl->wires) {
        wire->id = ++i;
    }
}

BitVar Circuit::getLiteral(bool b) const {
    return BitVar(b, pimpl_get_self());
}

Problem Circuit::generateCNF() const {
    number();
    Problem p;
    for (auto& gate : pimpl->gates) {
        gate->emplaceCNF(p);
    }
    if (pimpl->lit0->referenced()) {
        p.addClause({-(pimpl->lit0->getID())});
    }
    if (pimpl->lit1->referenced()) {
        p.addClause({pimpl->lit1->getID()});
    }
    if (pimpl->final_output) {
        p.addClause({pimpl->final_output->getID()});
    }
    return std::move(p);
}

std::shared_ptr<Circuit::Value> Circuit::getLiteralTrue(const std::weak_ptr<Circuit::impl>& c) {
    auto pimpl = c.lock();
    assert(pimpl);
    return std::make_shared<Value>(*(pimpl->lit1));
}

std::shared_ptr<Circuit::Value> Circuit::getLiteralFalse(const std::weak_ptr<Circuit::impl>& c) {
    auto pimpl = c.lock();
    assert(pimpl);
    return std::make_shared<Value>(*(pimpl->lit0));
}

std::shared_ptr<Circuit::Value> Circuit::getLiteralTrue() const {
    return std::make_shared<Value>(*(pimpl->lit1));
}

std::shared_ptr<Circuit::Value> Circuit::getLiteralFalse() const {
    return std::make_shared<Value>(*(pimpl->lit0));
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
