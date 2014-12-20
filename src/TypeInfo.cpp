#include <CXXSat/TypeInfo.h>

std::ostream& operator<<(std::ostream& os, const TypeInfo& info) {
    if (info.isBit()) {
        os << "bit";
    }
    else {
        if (info.sign()) {
            os << 's';
        }
        else {
            os << 'u';
        }
        os << info.size();
    }
    return os;
}
 
