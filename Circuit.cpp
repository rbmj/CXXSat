#include "Circuit.h"
#include "Gates.h"
#include "Variable.h"

struct return_value {
    std::shared_ptr<Variable> value;
    std::vector<BitVar> conditions;
};

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
    std::vector<return_value> return_values;
    std::shared_ptr<Value> final_output;
    std::unordered_set<std::shared_ptr<Argument>> arguments;
    std::shared_ptr<Input> lit0;
    std::shared_ptr<Input> lit1;
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

void Circuit::yield(const std::shared_ptr<Variable>& v, std::vector<BitVar>&& conds) {
    pimpl->return_values.push_back({v, std::move(conds)});
}

void Circuit::number() const {
    int i = 0;
    for (auto& wire : pimpl->wires) {
        wire->id = ++i;
    }
}
void Circuit::yield(const std::shared_ptr<Variable>& v) {
    yield(v, {});
}

void Circuit::constrain_equal(const std::shared_ptr<Variable>& v) {
    std::vector<std::shared_ptr<Value>> values;
    std::vector<std::shared_ptr<Value>> tmp;
    assert(pimpl->return_values.size() != 0);
    for (auto& rv : pimpl->return_values) {
        tmp.clear();
        tmp.push_back((*v == *(rv.value)).getBit());
        if (rv.conditions.size() != 0) {
            for (auto& cond : rv.conditions) {
                tmp.push_back(cond.getBit());
            }
            values.push_back(MultiAnd(tmp));
        }
        else {
            values.push_back(tmp[0]);
        }
    }
    if (values.size() != 1) {
        pimpl->final_output = MultiOr(values);
    }
    else {
        pimpl->final_output = values.at(0);
    }
}

void Circuit::constrain_equal(bool b) {
    constrain_equal(std::make_shared<BitVar>(b, *this));
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

std::shared_ptr<Circuit::Value> MultiAnd(const std::vector<std::shared_ptr<Circuit::Value>>& values) {
    assert(values.size() != 0);
    return std::make_shared<Circuit::Value>(MultiAndGate::create(values));
}

std::shared_ptr<Circuit::Value> MultiOr(const std::vector<std::shared_ptr<Circuit::Value>>& values) {
    assert(values.size() != 0);
    return std::make_shared<Circuit::Value>(MultiOrGate::create(values));
}

