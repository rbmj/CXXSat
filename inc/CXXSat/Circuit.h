#ifndef GRAPH_H_INC
#define GRAPH_H_INC

#include <iostream>

#include <vector>
#include <memory>
#include <utility>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <string>
#include <assert.h>

#include <CXXSat/Range.h>
#include <CXXSat/Sat.h>
#include <CXXSat/TypeInfo.h>

//From an efficiency standpoint, I'm not completely satisfied with
//the extent of shared_ptr<> use here.  I wish I could do more stuff
//with explicit ownership but at least at this point it's way too much
//bookkeeping...

class Argument;
class Variable;

template <bool, unsigned>
class IntVar;

class Circuit {
public:
    class Node;
    friend class Node;
    class Wire;
    friend class Wire;
    class Input;
    class Value;
    class Gate;
    class BasicWire;
    class InvertingWire;
    template <class T>
    class GateBase;
    //public for convenience, but is incomplete, so no problems
    struct impl; 
private:
    //needs to be a shared_ptr in order to use weak_ptr<> (sad face)
    std::shared_ptr<impl> pimpl;
    const std::weak_ptr<impl>& pimpl_get_self() const;
public:
    Circuit();
    Argument addArgument(TypeInfo info);
    template <class Int>
    Argument addArgument();
    Argument addArgumentBit();
    static Value getLiteralTrue(const std::weak_ptr<Circuit::impl>&);
    static Value getLiteralFalse(const std::weak_ptr<Circuit::impl>&);
    Value getLiteralTrue() const;
    Value getLiteralFalse() const;
    Variable getLiteralBit(bool) const;
    template <class Int>
    Variable getLiteral(Int i) const;
    Problem generateCNF() const;
    Problem generateCNF(const Variable&) const;
};

static inline bool circuitsEqual(const std::weak_ptr<Circuit::impl>& a,
        const std::weak_ptr<Circuit::impl>& b)
{
    auto x = a.lock();
    auto y = b.lock();
    return (x && y && x == y) || (!x && !y);
}

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
    virtual ~Wire();
    virtual int ID() const = 0;
    void connect(const Node* n) {
        //EVIL!
        auto x = to.emplace(const_cast<Node*>(n), 1);
        if (!x.second) {
            ++(x.first->second);
        }
    }
    void disconnect(const Node* n) {
        auto it = to.find(const_cast<Node*>(n));
        assert(it != to.end());
        if (--(it->second) == 0) {
            to.erase(it);
        }
    }
    const std::weak_ptr<Circuit::impl>& getCircuit() {
        return c;
    }
    void swapSource(std::shared_ptr<Node>& p) {
        std::swap(from, p);
    }
protected:
    std::shared_ptr<Node> from;
    std::map<Node*, unsigned> to;
    std::weak_ptr<Circuit::impl> c;
    virtual bool setID(int) = 0;
};

class Circuit::BasicWire : public Circuit::Wire {
public:
    using Circuit::Wire::Wire;
    int ID() const {
        return id;
    }
    bool setID(int x) {
        id = x;
        return true;
    }
private:
    int id = 0;
};

class Circuit::InvertingWire : public Circuit::Wire {
public:
    using Circuit::Wire::Wire;
    int ID() const;
    bool setID(int) {
        return false;
    }
};

class Circuit::Input : public Circuit::Node {
public:
    static std::shared_ptr<Input> create(std::weak_ptr<Circuit::impl> c) {
        auto ptr = std::make_shared<Input>(std::move(c));
        ptr->self = ptr;
        return ptr;
    }
    bool referenced() const {
        return initialized && !(out_wire.expired());
    }
    std::shared_ptr<Wire> getWire() const {
        if (auto wire = out_wire.lock()) {
            return wire;
        }
        else {
            auto ptr = self.lock();
            assert(ptr);
            auto ret = std::make_shared<BasicWire>(ptr);
            out_wire = ret;
            initialized = true;
            return ret;
        }
    }
    int getID() const {
        return getWire()->ID();
    }
    //DO NOT USE
    explicit Input(std::weak_ptr<Circuit::impl> c) : Node(c, NODE_TYPE::INPUT), initialized(false) {}
private:
    mutable std::weak_ptr<Wire> out_wire;
    mutable bool initialized;
    std::weak_ptr<Node> self;
};

class Circuit::Value : public Circuit::Node {
public:
    Value() : Node({}, NODE_TYPE::VALUE) {}
    Value(const Value& v) 
        : Node(v.circuit, NODE_TYPE::VALUE), _source(v._source)
    {
        _source->connect(this);
    }
    Value(Value&& v) : Node(v.circuit, NODE_TYPE::VALUE) {
        std::swap(v._source, _source);
        _source->disconnect(&v);
        _source->connect(this);
    }
    explicit Value(const Input& i) 
        : Node(i.getCircuit(), NODE_TYPE::VALUE), _source(i.getWire())
    {
        _source->connect(this);
    }
    explicit Value(std::shared_ptr<Wire> w)
        : Node(w->getCircuit(), NODE_TYPE::VALUE), _source(w)
    {
        _source->connect(this);
    }
    Value& operator=(const Value& v) {
        if (_source) {
            _source->disconnect(this);
        }
        circuit = v.circuit;
        _source = v._source;
        if (_source) {
            _source->connect(this);
        }
        return *this;
    }
    Value& operator=(Value&& v) {
        if (_source) {
            _source->disconnect(this);
            _source.reset();
        }
        circuit = v.circuit;
        std::swap(_source, v._source);
        if (_source) {
            _source->connect(this);
            _source->disconnect(&v);
        }
        return *this;
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
        if (_source) _source->disconnect(this);
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
            throw 0; //we should not exist if there is no wire...
        }
    }
private:
    explicit Gate(const std::weak_ptr<Circuit::impl>& c)
        : Node(c, NODE_TYPE::GATE) {}
protected:
    std::weak_ptr<Wire> out_wire;
    void init(std::weak_ptr<Wire> w) {
        out_wire = w;
    }
};

//MAINTAINER'S NOTE:  see Argument.h for AddArgument<>() impl,
//                        Variable.h for getLiteral<>() impl.

#endif
