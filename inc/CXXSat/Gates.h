#ifndef GATES_H_INC
#define GATES_H_INC

#include <CXXSat/Circuit.h>
#include <assert.h>

template <class DerivedGate>
class Circuit::GateBase : public Circuit::Gate {
public:
    template <class... Args>
    static std::shared_ptr<Circuit::Wire> create(Args&&... args) {
        auto dg = std::make_shared<DerivedGate>(std::forward<Args>(args)...);
        auto wire = std::make_shared<BasicWire>(dg);
        dg->init(wire);
        return wire;
    }
protected:
    GateBase(const std::weak_ptr<Circuit::impl>& c) : Gate(c) {}
    virtual ~GateBase() {}
};

class NotGate : public Circuit::GateBase<NotGate> {
    friend class Circuit::GateBase<NotGate>;
    friend class Circuit::InvertingWire;
public:
    ~NotGate() {
        source->disconnect(this);
    }
    static std::shared_ptr<Circuit::Wire> create(const Circuit::Value&);
    void emplaceCNF(Problem& p);
    //DO NOT USE:
    NotGate(const Circuit::Value& _a) : GateBase<NotGate>(_a.getCircuit()) {
        source = _a.source();
        source->connect(this);
    }
private:
    std::shared_ptr<Circuit::Wire> source;
};

template <class DerivedGate>
class BinaryGate : public Circuit::GateBase<DerivedGate> {
protected:
    std::shared_ptr<Circuit::Wire> a;
    std::shared_ptr<Circuit::Wire> b;
public:
    BinaryGate(const Circuit::Value& _a, const Circuit::Value& _b)
        : Circuit::GateBase<DerivedGate>(_a.getCircuit())
    {
        assert(circuitsEqual(_a.getCircuit(), _b.getCircuit()));
        a = _a.source();
        b = _b.source();
        a->connect(this);
        b->connect(this);
    }
    BinaryGate(std::shared_ptr<Circuit::Wire> _a, std::shared_ptr<Circuit::Wire> _b)
        : Circuit::GateBase<DerivedGate>(_a->getCircuit()),
          a(std::move(_a)), b(std::move(_b))
    {
        a->connect(this);
        b->connect(this);
    }
    virtual ~BinaryGate() {
        a->disconnect(this);
        b->disconnect(this);
    }
    template <class OtherGate>
    typename std::enable_if<std::is_base_of<BinaryGate, OtherGate>::value>::type
    /* void */ replaceWith() {
        auto out = this->out_wire.lock();
        assert(out);
        auto new_gate = std::make_shared<OtherGate>(a, b);
        out.swapSource(new_gate);
    }
};

#define DECLARE_BINARY_GATE(name) \
    class name : public BinaryGate<name> { \
    public: \
        void emplaceCNF(Problem& p); \
        using BinaryGate<name>::BinaryGate; \
    }

DECLARE_BINARY_GATE(AndGate);
DECLARE_BINARY_GATE(NandGate);
DECLARE_BINARY_GATE(OrGate);
DECLARE_BINARY_GATE(NorGate);
DECLARE_BINARY_GATE(XorGate);
DECLARE_BINARY_GATE(XnorGate);

#undef DECLARE_BINARY_GATE

template <class DerivedGate>
class MultiGate : public Circuit::GateBase<DerivedGate> {
public:
    /* commented due to not needing right now, and compile error
     * with init-list::iterator defined as pointer to reference...
    template <class... Args>
    MultiGate(const Circuit::Value& _a, Args&&... args) 
        : Circuit::GateBase<DerivedGate>(_a.getCircuit())
    {
        std::initializer_list<const Circuit::Value&> values{_a, args...};
        std::transform(begin(values), end(values), std::inserter(inputs, begin(inputs)),
                [](const Circuit::Value& c) { return c.source(); });
        for (auto& i : inputs) {
            i->connect(this);
        }
    }
    */
    template <class Container>
    MultiGate(const Container& args)
        : Circuit::GateBase<DerivedGate>(begin(args)->getCircuit())
    {
        std::transform(begin(args), end(args), std::inserter(inputs, begin(inputs)),
            [](const Circuit::Value& v) {
                return v.source();
            }
        );
        for (auto& i : inputs) {
            i->connect(this);
        }
    }
    ~MultiGate() {
        for (auto& i : inputs) {
            i->disconnect(this);
        }
    }
protected:
    std::vector<std::shared_ptr<Circuit::Wire>> inputs;
};

#define DECLARE_MULTI_GATE(name) \
    class name : public MultiGate<name> { \
    public: \
        void emplaceCNF(Problem& p); \
        using MultiGate<name>::MultiGate; \
    }

DECLARE_MULTI_GATE(MultiAndGate);
DECLARE_MULTI_GATE(MultiOrGate);

#undef DECLARE_MULTI_GATE

//capital first letters to not conflict with reserved words (and, or).
//all are cap for consistency
Circuit::Value And(const Circuit::Value&, const Circuit::Value&);
Circuit::Value Nand(const Circuit::Value&, const Circuit::Value&);
Circuit::Value Or(const Circuit::Value&, const Circuit::Value&);
Circuit::Value Nor(const Circuit::Value&, const Circuit::Value&);
Circuit::Value Xor(const Circuit::Value&, const Circuit::Value&);
Circuit::Value Xnor(const Circuit::Value&, const Circuit::Value&);
Circuit::Value Not(const Circuit::Value&);

template <class Container>
Circuit::Value MultiAnd(const Container& values) {
    return Circuit::Value{MultiAndGate::create(values)};
}

template <class Container>
Circuit::Value MultiOr(const Container& values) {
    return Circuit::Value{MultiOrGate::create(values)};
}

typedef std::pair<Circuit::Value, Circuit::Value> AdderResT;
AdderResT FullAdder(
        const Circuit::Value& a,
        const Circuit::Value& b,
        const Circuit::Value& carry);

#endif
