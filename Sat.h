#ifndef SAT_H_INC
#define SAT_H_INC

#include <vector>
#include <string>
#include <ostream>

typedef std::vector<int> Clause;
typedef std::initializer_list<int> Clause_list;

class Problem {
private:
    std::vector<Clause> clauses;
    unsigned max_var = 0;
public:
    typedef std::vector<Clause>::iterator iterator;
    typedef std::vector<Clause>::const_iterator const_iterator;
    void addClause(Clause);
    void addClause(Clause_list l) {
        addClause(Clause(l));
    }
    std::string toDIMACS() const;
    void printDIMACS(std::ostream& o) const;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    void solve() const;
};

#endif
