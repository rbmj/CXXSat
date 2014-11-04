#ifndef SAT_H_INC
#define SAT_H_INC

#include <vector>
#include <string>
#include <ostream>
#include <unordered_map>
#include <memory>

typedef std::vector<int> Clause;
typedef std::initializer_list<int> Clause_list;
class Solution;

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
    Solution solve(bool = false) const;
};

class Solution {
    friend class Problem;
private:
    typedef std::unordered_map<int, bool> varmap_t;
    std::unique_ptr<varmap_t> varmap;
    Solution(std::unique_ptr<varmap_t>&& a) : varmap(std::move(a)) {}
    Solution() = default;
public:
    Solution(const Solution&) = default;
    Solution(Solution&&) = default;
    bool operator[](int i) const {
        return (*varmap)[i];
    }
    bool at(int i) const {
        return varmap->at(i);
    }
    explicit operator bool() const {
        return (bool)varmap;
    }
};

#endif
