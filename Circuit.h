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

//From an efficiency standpoint, I'm not completely satisfied with
//the extent of shared_ptr<> use here.  I wish I could do more stuff
//with explicit ownership but at least at this point it's way too much
//bookkeeping...

class Argument;
class Variable;

class Circuit {
public:
    class Node;
    friend class Node;
    class Wire;
    friend class Wire;
    class Input;
    class Value;
    class Gate;
    template <class T>
    class GateBase;
    //public for convenience, but is incomplete, so no problems
    struct impl; 
private:
    //needs to be a shared_ptr in order to use weak_ptr<> (sad face)
    std::shared_ptr<impl> pimpl;
    void pimpl_emplace_argument(std::string, std::shared_ptr<Argument>);
    const std::weak_ptr<impl>& pimpl_get_self() const;
public:
    Circuit();
    template <class T, class... Args>
    std::shared_ptr<T> addArgument(const std::string& s, Args&&... args) {
        auto ptr = std::make_shared<T>(
                pimpl_get_self(), std::forward<Args>(args)...);
        pimpl_emplace_argument(std::move(s), ptr);
        return ptr;
    }
    /*
    template <class T, class U>
    const T& getLiteral(U u) {
        return T::getLiteral(u, lit0, lit1);
    }
    */
    void yield(const std::shared_ptr<Variable>& v);
    void number();
    Problem generateCNF() const;
};

class Circuit::Node {
protected:
    enum class NODE_TYPE : char {
        INPUT,
        VALUE,
        GATE
    };
    std::weak_ptr<Circuit::impl> circuit;
    NODE_TYPE type;
public:
    Node(const std::weak_ptr<Circuit::impl>& c, NODE_TYPE t);
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
    const std::weak_ptr<Circuit::impl>& getCircuit() const {
        return circuit;
    }
};

class Circuit::Wire {
    friend class Circuit;
public:
    explicit Wire(const std::shared_ptr<Node>& n);
    ~Wire();
    int ID() { return id; }
    void connect(Node* n) {
        to.push_back(n);
    }
    void disconnect(Node* n) {
        to.erase(std::remove(to.begin(), to.end(), n));
    }
    const std::weak_ptr<Circuit::impl>& getCircuit() {
        return c;
    }
    void swapSource(std::shared_ptr<Node>& p) {
        std::swap(from, p);
    }
private:
    std::shared_ptr<Node> from;
    std::vector<Node*> to;
    std::weak_ptr<Circuit::impl> c;
    int id;
};

class Circuit::Input : public Circuit::Node {
public:
    static std::shared_ptr<Input> create(std::weak_ptr<Circuit::impl> c) {
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
    explicit Input(std::weak_ptr<Circuit::impl> c) : Node(c, NODE_TYPE::INPUT) {}
private:
    mutable std::weak_ptr<Wire> out_wire;
    std::weak_ptr<Node> self;
};

class Circuit::Value : public Circuit::Node {
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

class Circuit::Gate : public Circuit::Node {
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
    explicit Gate(const std::weak_ptr<Circuit::impl>& c)
        : Node(c, NODE_TYPE::GATE) {}
    void init(std::weak_ptr<Wire> w) {
        out_wire = w;
    }
protected:
    std::weak_ptr<Wire> out_wire;
};

//capital first letters to not conflict with reserved words (and, or).
//all are cap for consistency
std::shared_ptr<Circuit::Value> And(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Nand(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Or(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Nor(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Xor(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Xnor(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Not(std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> MultiAnd(const std::vector<std::shared_ptr<Circuit::Value>>&);
std::shared_ptr<Circuit::Value> MultiOr(const std::vector<std::shared_ptr<Circuit::Value>>&);

#endif
