#include "Sat.h"

#include <sstream>
#include <string>
#include <iostream>

//later do something a bit more flexible
//dependency injection or something of the sort
#include <minisat/core/Solver.h>
#include <minisat/core/SolverTypes.h>

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

Solution Problem::solve() const {
    Minisat::Solver s;
    Minisat::vec<Minisat::Lit> lits;
    for (auto& clause : clauses) {
        for (auto& var_in : clause) {
            unsigned var = ((var_in > 0) ? var_in : -var_in) - 1;
            while (var >= s.nVars()) {
                s.newVar();
            }
            lits.push((var_in > 0) ?
                   Minisat::mkLit(var) :
                   ~Minisat::mkLit(var));
        }
        s.addClause(lits);
        lits.clear();
    }
    if (!s.simplify()) {
        std::cout << "UNSAT\n";
    }
    //perhaps use solveLimited here for resource constraints later
    if (s.solve()) {
        auto solution = std::make_unique<std::unordered_map<int, bool>>();
        for (int i = 0; i < s.nVars(); ++i) {
            if (s.model[i] != Minisat::l_Undef) {
                if (s.model[i] != Minisat::l_True) {
                    solution->insert({i, false});
                }
                else {
                    solution->insert({i, true});
                }
            }
        }
        return Solution(std::move(solution));
    }
    else {
        return {};
    }
}

