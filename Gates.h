#ifndef GATES_H_INC
#define GATES_H_INC

#include "Circuit.h"
#include <assert.h>

template <class DerivedGate>
class GateBase : public Gate {
public:
    template <class... Args>
    static std::shared_ptr<Wire> create(Args&&... args) {
        auto dg = std::make_shared<DerivedGate>(std::forward<Args>(args)...);
        auto wire = std::make_shared<Wire>(dg);
        dg->init(wire);
        return wire;
    }
protected:
    GateBase(Circuit* c) : Gate(c) {}
    virtual ~GateBase() {}
};

class NotGate : public GateBase<NotGate> {
    friend class GateBase<NotGate>;
public:
    ~NotGate() {
        a->disconnect(this);
    }
    void emplaceCNF(Problem& p);
    //DO NOT USE:
    NotGate(const Value& _a) : GateBase<NotGate>(_a.getCircuit()) {
        a = _a.source();
        a->connect(this);
    }
private:
    std::shared_ptr<Wire> a;
};
        
template <class DerivedGate>
class BinaryGate : public GateBase<DerivedGate> {
protected:
    std::shared_ptr<Wire> a;
    std::shared_ptr<Wire> b;
public:
    BinaryGate(const Value& _a, const Value& _b)
        : GateBase<DerivedGate>(_a.getCircuit())
    {
        assert(_a.getCircuit() == _b.getCircuit());
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

