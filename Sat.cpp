#include "Sat.h"

#include <iostream>

void printDIMACS(const Problem& p) {
    std::cout << "p cnf NUMVARS NUMCLAUSES\n";
    for (auto& clause : p) {
        for (auto& lit : clause) {
            std::cout << lit << ' ';
        }
        std::cout << "0\n";
    }
}
