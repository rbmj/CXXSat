#include <CXXSat/Sat.h>

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

Solution Problem::solve(bool debug) const {
    Minisat::Solver s;
    Minisat::vec<Minisat::Lit> lits;
    for (auto& clause : clauses) {
        for (auto& var_in : clause) {
            int var = ((var_in > 0) ? var_in : -var_in) - 1;
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
    //perhaps use solveLimited here for resource constraints later
    if (s.simplify() && s.solve()) {
        auto solution = std::make_unique<std::unordered_map<int, bool>>();
        for (int i = 0; i < s.nVars();) {
            if (s.model[i] != Minisat::l_Undef) {
                //Note the ++ in this - to avoid off-by-one errors
                if (s.model[i] != Minisat::l_True) {
                    solution->insert({++i, false});
                    if (debug) std::cout << -i << ' ';
                }
                else {
                    solution->insert({++i, true});
                    if (debug) std::cout << i << ' ';
                }
            }
        }
        if (debug) std::cout << '\n';
        return Solution(std::move(solution));
    }
    else {
        return {};
    }
}

