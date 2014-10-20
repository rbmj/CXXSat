#include "Gates.h"

// From http://en.wikipedia.org/wiki/Tseitin_transformation

void NotGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto C = getWire()->ID();
    p.emplace_back(CLAUSE{-A, -C});
    p.emplace_back(CLAUSE{A, C});
}

void AndGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.emplace_back(CLAUSE{-A, -B, C});
    p.emplace_back(CLAUSE{A, -C});
    p.emplace_back(CLAUSE{B, -C});
}

void NandGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.emplace_back(CLAUSE{-A, -B, -C});
    p.emplace_back(CLAUSE{A, C});
    p.emplace_back(CLAUSE{B, C});
}

void OrGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.emplace_back(CLAUSE{A, B, -C});
    p.emplace_back(CLAUSE{-A, C});
    p.emplace_back(CLAUSE{-B, C});
}

void NorGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.emplace_back(CLAUSE{A, B, C});
    p.emplace_back(CLAUSE{-A, -C});
    p.emplace_back(CLAUSE{-B, -C});
}

void XorGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.emplace_back(CLAUSE{-A, -B, -C});
    p.emplace_back(CLAUSE{A, B, -C});
    p.emplace_back(CLAUSE{A, -B, C});
    p.emplace_back(CLAUSE{-A, B, C});
}

void NxorGate::emplaceCNF(Problem& p) {
    auto A = a->ID();
    auto B = b->ID();
    auto C = getWire()->ID();
    p.emplace_back(CLAUSE{-A, -B, C});
    p.emplace_back(CLAUSE{A, B, C});
    p.emplace_back(CLAUSE{A, -B, -C});
    p.emplace_back(CLAUSE{-A, B, -C});
}

