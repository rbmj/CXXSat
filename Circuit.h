#ifndef GRAPH_H_INC
#define GRAPH_H_INC

#include <iostream>


#include <vector>
#include <memory>
#include <utility>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <assert.h>

#include "Range.h"
#include "Sat.h"
#include "Argument.h"
#include "Variable.h"

class Node;
class Wire;
class Input;
class Value;
class Gate;

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
    //Order of declaration is important (unfortunately)
    //These must be declared first, so that they are destroyed
    std::unordered_set<Input*> inputs;
    std::unordered_set<Value*> outputs;
    std::unordered_set<Gate*> gates;
    std::unordered_set<Wire*> wires;
    //declare these after
    std::vector<std::shared_ptr<Variable>> return_values;
    std::unordered_map<std::string, std::shared_ptr<Argument>> arguments;
    std::shared_ptr<Input> lit0;
    std::shared_ptr<Input> lit1;
    std::weak_ptr<Circuit> self;
public:
    Circuit() = default;
    static std::shared_ptr<Circuit> create() {
        auto ptr = std::make_shared<Circuit>();
        ptr->self = ptr;
        return std::move(ptr);
    }
    template <class T, class... Args>
    std::shared_ptr<T> addArgument(std::string s, Args&&... args) {
        auto ptr = std::make_shared<T>(self, std::forward<Args>(args)...);
        arguments.emplace(std::move(s), ptr);
        return ptr;
    }
    template <class T, class U>
    const T& getLiteral(U u) {
        return T::getLiteral(u, lit0, lit1);
    }
    void yield(const std::shared_ptr<Variable>& v) {
        return_values.push_back(v);
    }
    void number();
    Problem generateCNF() const;
};

class Node {
protected:
    enum class NODE_TYPE : char {
        INPUT,
        VALUE,
        GATE
    };
    std::weak_ptr<Circuit> circuit;
    NODE_TYPE type;
public:
    Node(const std::weak_ptr<Circuit>& c, NODE_TYPE t);
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
    const std::weak_ptr<Circuit>& getCircuit() const {
        return circuit;
    }
};

class Wire {
    friend class Circuit;
public:
    explicit Wire(std::shared_ptr<Node> n) 
        : from(n), c(n->getCircuit()), id(0) 
    {
        if (auto circuit = c.lock()) {
            circuit->reg(this);
        }
    }
    ~Wire() {
        if (auto circuit = c.lock()) {
            circuit->unreg(this);
        }
    }
    int ID() { return id; }
    void connect(Node* n) {
        to.push_back(n);
    }
    void disconnect(Node* n) {
        to.erase(std::remove(to.begin(), to.end(), n));
    }
    const std::weak_ptr<Circuit>& getCircuit() {
        return c;
    }
private:
    std::shared_ptr<Node> from;
    std::vector<Node*> to;
    std::weak_ptr<Circuit> c;
    int id;
};

class Input : public Node {
public:
    static std::shared_ptr<Input> create(std::weak_ptr<Circuit> c) {
        auto ptr = std::make_shared<Input>(std::move(c));
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
            out_wire = ret;
            return ret;
        }
    }
    int getID() const {
        return getWire()->ID();
    }
    //DO NOT USE
    explicit Input(std::weak_ptr<Circuit> c) : Node(c, NODE_TYPE::INPUT) {}
private:
    mutable std::weak_ptr<Wire> out_wire;
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
    static std::shared_ptr<Value> create(const std::shared_ptr<Value>& v) {
        return std::make_shared<Value>(*v);
    }
    static std::shared_ptr<Value> create(const std::shared_ptr<Input>& i) {
        return std::make_shared<Value>(*i);
    }
    std::shared_ptr<Value> clone() const {
        return std::make_shared<Value>(*this);
    }
    std::shared_ptr<Wire> source() const {
        return _source;
    }
    ~Value() {
        _source->disconnect(this);
    }
    int getID() const {
        return _source->ID();
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
    explicit Gate(const std::weak_ptr<Circuit>& c)
        : Node(c, NODE_TYPE::GATE) {}
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
