#include "Sat.h"

#include <sstream>

void Problem::addClause(Clause c) {
    //this is not the ideal way to keep track of this...
    for (auto& x : c) {
        unsigned y = (x >= 0) ? x : -x;
        if (y > max_var) {
            max_var = y;
        }
    }
    clauses.push_back(std::move(c));
}

std::string Problem::toDIMACS() const {
    std::ostringstream ss;
    printDIMACS(ss);
    return ss.str();
}

void Problem::printDIMACS(std::ostream& s) const {
    s << "p cnf " << max_var << ' ' << clauses.size() << '\n';
    for (auto& clause : clauses) {
        for (auto& lit : clause) {
            s << lit << ' ';
        }
        s << "0\n";
    }
}

