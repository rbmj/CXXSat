#include <CXXSat/TypeInfo.h>
#include <sstream>

std::string TypeInfo::toString() const {
    std::ostringstream str;
    str << *this;
    return str.str();
}

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
 
