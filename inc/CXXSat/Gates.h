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
        auto wire = std::make_shared<Wire>(dg);
        dg->init(wire);
        return wire;
    }
protected:
    GateBase(const std::weak_ptr<Circuit::impl>& c) : Gate(c) {}
    virtual ~GateBase() {}
};

class NotGate : public Circuit::GateBase<NotGate> {
    friend class Circuit::GateBase<NotGate>;
public:
    ~NotGate() {
        a->disconnect(this);
    }
    void emplaceCNF(Problem& p);
    //DO NOT USE:
    NotGate(const Circuit::Value& _a) : GateBase<NotGate>(_a.getCircuit()) {
        a = _a.source();
        a->connect(this);
    }
private:
    std::shared_ptr<Circuit::Wire> a;
};

static inline bool circuitsEqual(const std::weak_ptr<Circuit::impl>& a,
        const std::weak_ptr<Circuit::impl>& b)
{
    auto x = a.lock();
    auto y = b.lock();
    return (x && y && x == y) || (!x && !y);
}

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
        : Circuit::GateBase<DerivedGate>((*(begin(args)))->getCircuit())
    {
        std::transform(begin(args), end(args), std::inserter(inputs, begin(inputs)),
            [](const std::shared_ptr<Circuit::Value>& v) {
                return v->source();
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
std::shared_ptr<Circuit::Value> And(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Nand(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Or(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Nor(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Xor(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Xnor(std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>);
std::shared_ptr<Circuit::Value> Not(std::shared_ptr<Circuit::Value>);

template <class Container>
std::shared_ptr<Circuit::Value> MultiAnd(const Container& values) {
    return std::make_shared<Circuit::Value>(MultiAndGate::create(values));
}

template <class Container>
std::shared_ptr<Circuit::Value> MultiOr(const Container& values) {
    return std::make_shared<Circuit::Value>(MultiOrGate::create(values));
}

typedef std::pair<std::shared_ptr<Circuit::Value>, std::shared_ptr<Circuit::Value>> AdderResT;
AdderResT FullAdder(
        std::shared_ptr<Circuit::Value> a,
        std::shared_ptr<Circuit::Value> b,
        std::shared_ptr<Circuit::Value> carry);

#endif
