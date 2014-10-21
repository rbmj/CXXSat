#ifndef SAT_H_INC
#define SAT_H_INC

#include <vector>

typedef std::vector<int> Clause;
typedef std::vector<Clause> Problem;

void printDIMACS(const Problem& p);

#define CLAUSE std::initializer_list<int>

#endif
