#ifndef GATES_H_INC
#define GATES_H_INC

#include "Circuit.h"
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
    virtual ~BinaryGate() {
        a->disconnect(this);
        b->disconnect(this);
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
DECLARE_BINARY_GATE(NxorGate);

#undef DECLARE_BINARY_GATE

#endif

