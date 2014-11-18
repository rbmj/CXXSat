#include <CXXSat/Circuit.h>
#include <CXXSat/Gates.h>
#include <CXXSat/Variable.h>

#include "CircuitImpl.h"


Circuit::Circuit() {
    pimpl = std::make_shared<Circuit::impl>();
    pimpl->self = pimpl;
    pimpl->lit0 = Input::create(pimpl);
    pimpl->lit1 = Input::create(pimpl);
}

const std::weak_ptr<Circuit::impl>& Circuit::pimpl_get_self() const {
    return pimpl->self;
}

Problem Circuit::generateCNF(const BitVar& b) const {
    auto cnf = generateCNF();
    cnf.addClause({b.getBit().getID()});
    return std::move(cnf);
}

void Circuit::impl::number() {
    int i = 1;
    for (auto& wire : wires) {
        if (wire->setID(i)) ++i;
    }
}

BitVar Circuit::getLiteral(bool b) const {
    return BitVar(b, pimpl_get_self());
}

Problem Circuit::generateCNF() const {
    return pimpl->generateCNF();
}

Problem Circuit::impl::generateCNF() {
    number();
    Problem p;
    for (auto& gate : gates) {
        gate->emplaceCNF(p);
    }
    if (lit0->referenced()) {
        p.addClause({-(lit0->getID())});
    }
    if (lit1->referenced()) {
        p.addClause({lit1->getID()});
    }
    return std::move(p);
}

Circuit::Value Circuit::getLiteralTrue(const std::weak_ptr<Circuit::impl>& c) {
    auto pimpl = c.lock();
    assert(pimpl);
    return Circuit::Value(*(pimpl->lit1));
}

Circuit::Value Circuit::getLiteralFalse(const std::weak_ptr<Circuit::impl>& c) {
    auto pimpl = c.lock();
    assert(pimpl);
    return Circuit::Value(*(pimpl->lit0));
}

Circuit::Value Circuit::getLiteralTrue() const {
    return Circuit::Value(*(pimpl->lit1));
}

Circuit::Value Circuit::getLiteralFalse() const {
    return Circuit::Value(*(pimpl->lit0));
}

Circuit::Node::Node(const std::weak_ptr<Circuit::impl>& c, Node::NODE_TYPE t)
    : circuit(c), type(t) 
{
    if (auto ptr = circuit.lock()) {
        switch (type) {
            case NODE_TYPE::INPUT:
                /*
                ptr->reg(asInput());
                */
                break;
            case NODE_TYPE::VALUE:
                /* don't register
                ptr->reg(asValue());
                */
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
                /*
                ptr->unreg(asInput());
                */
                break;
            case NODE_TYPE::VALUE:
                /*
                ptr->unreg(asValue());
                */
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
    : from(n), c(n->getCircuit()) 
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

int Circuit::InvertingWire::ID() const {
    return -(((NotGate&)(*from)).source->ID());
}
