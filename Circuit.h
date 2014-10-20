#ifndef GRAPH_H_INC
#define GRAPH_H_INC

#include <vector>
#include <memory>
#include <utility>
#include <algorithm>
#include <unordered_set>
#include <assert.h>

#include "Range.h"
#include "Sat.h"

class Node;
class Wire;
class Input;
class Value;
class Gate;
class Variable;
class Argument;

class Circuit {
    friend class Node;
    friend class Wire;
private:
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
    std::unique_ptr<Variable> return_value;
    std::vector<std::unique_ptr<Argument>> arguments;
    std::unordered_set<Input*> inputs;
    std::unordered_set<Value*> outputs;
    std::unordered_set<Gate*> gates;
    std::unordered_set<Wire*> wires;
};

class Node {
protected:
    enum class NODE_TYPE : char {
        INPUT,
        VALUE,
        GATE
    };
    Circuit * circuit;
    NODE_TYPE type;
public:
    Node(Circuit* c, NODE_TYPE t);
    virtual ~Node() = 0;
    bool isInput() const {
        return type == NODE_TYPE::INPUT;
    }
    const Input* asInput() const {
        return (const Input*)this;
    }
    Input* asInput() {
        return (Input*)this;
    }
    bool isValue() const {
        return type == NODE_TYPE::VALUE;
    }
    const Value* asValue() const {
        return (const Value*)this;
    }
    Value* asValue() {
        return (Value*)this;
    }
    bool isGate() const {
        return type == NODE_TYPE::GATE;
    }
    const Gate* asGate() const {
        return (const Gate*)this;
    }
    Gate* asGate() {
        return (Gate*)this;
    }
    Circuit* getCircuit() const {
        return circuit;
    }
};

class Wire {
    friend class Circuit;
public:
    explicit Wire(std::shared_ptr<Node> n) 
        : from(n), c(n->getCircuit()), id(0) 
    {
        c->reg(this);
    }
    ~Wire() {
        c->unreg(this);
    }
    int ID();
    void connect(Node* n) {
        to.push_back(n);
    }
    void disconnect(Node* n) {
        to.erase(std::remove(to.begin(), to.end(), n));
    }
    Circuit* getCircuit() {
        return c;
    }
private:
    std::shared_ptr<Node> from;
    std::vector<Node*> to;
    Circuit* c;
    int id;
};

class Input : public Node {
public:
    static std::shared_ptr<Input> create(Circuit* c) {
        auto ptr = std::make_shared<Input>(c);
        ptr->self = ptr;
        return ptr;
    }
    bool referenced() const {
        return !(out_wire.expired());
    }
    std::shared_ptr<Wire> getWire() const {
        if (auto wire = out_wire.lock()) {
            return wire;
        }
        else {
            auto ptr = self.lock();
            assert(ptr);
            auto ret = std::make_shared<Wire>(ptr);
            wire = ret;
            return ret;
        }
    }
    explicit Input(Circuit* c) : Node(c, NODE_TYPE::INPUT) {} //DO NOT USE
private:
    std::weak_ptr<Wire> out_wire;
    std::weak_ptr<Node> self;
};

class Value : public Node {
public:
    Value(const Value& v) 
        : Node(v.circuit, NODE_TYPE::VALUE), _source(v._source)
    {
        _source->connect(this);
    }
    explicit Value(const Input& i) 
        : Node(i.getCircuit(), NODE_TYPE::VALUE), _source(i.getWire())
    {
        _source->connect(this);
    }
    Value(std::shared_ptr<Wire> w)
        : Node(w->getCircuit(), NODE_TYPE::VALUE), _source(w)
    {
        _source->connect(this);
    }
    std::shared_ptr<Wire> source() const {
        return _source;
    }
private:
    std::shared_ptr<Wire> _source;
};

template <class T>
class GateBase;

class Gate : public Node {
    template <class T>
    friend class GateBase;
public:
    virtual void emplaceCNF(Problem& p) = 0;
    virtual Problem CNF() {
        Problem ret;
        emplaceCNF(ret);
        return ret;
    }
    std::shared_ptr<Wire> getWire() {
        if (auto wire = out_wire.lock()) {
            return wire;
        }
        else {
            assert(false); //we should not exist if there is no wire...
        }
    }
private:
    explicit Gate(Circuit* c) : Node(c, NODE_TYPE::GATE) {}
    void init(std::weak_ptr<Wire> w) {
        out_wire = w;
    }
    std::weak_ptr<Wire> out_wire;
};

//capital first letters to not conflict with reserved words (and, or).
//all are cap for consistency
std::shared_ptr<Value> And(std::shared_ptr<Value>, std::shared_ptr<Value>);
std::shared_ptr<Value> Nand(std::shared_ptr<Value>, std::shared_ptr<Value>);
std::shared_ptr<Value> Or(std::shared_ptr<Value>, std::shared_ptr<Value>);
std::shared_ptr<Value> Nor(std::shared_ptr<Value>, std::shared_ptr<Value>);
std::shared_ptr<Value> Xor(std::shared_ptr<Value>, std::shared_ptr<Value>);
std::shared_ptr<Value> Nxor(std::shared_ptr<Value>, std::shared_ptr<Value>);
std::shared_ptr<Value> Not(std::shared_ptr<Value>);

#endif
