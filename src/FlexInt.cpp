#include <CXXSat/FlexInt.h>

//clang needs to expand a lot of templates that end up not getting used, so we'll ignore the
//spurrious warnings.  There's an assert in do_binop that will catch us if there's an error.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"

FlexInt operator+(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x+y}; });
}
FlexInt operator-(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x-y}; });
}
FlexInt operator*(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x*y}; });
}
FlexInt operator/(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x/y}; });
}
FlexInt operator%(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x%y}; });
}
FlexInt operator>>(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x>>y}; });
}
FlexInt operator<<(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x<<y}; });
}
FlexInt operator&(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x&y}; });
}
FlexInt operator|(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x|y}; });
}
FlexInt operator^(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return FlexInt{x^y}; });
}
bool operator<(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return bool{x<y}; });
}
bool operator>(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return bool{x>y}; });
}
bool operator<=(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return bool{x<=y}; });
}
bool operator>=(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return bool{x>=y}; });
}
bool operator==(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return bool{x==y}; });
}
bool operator!=(const FlexInt& a, const FlexInt& b) {
    return FlexInt::do_binop(a, b, [](const auto& x, const auto& y) { return bool{x!=y}; });
}

#pragma clang diagnostic pop

std::ostream& operator<<(std::ostream& out, const FlexInt& i) {
    i.do_t([&out](const auto& x) { out << x; });
    return out;
}

