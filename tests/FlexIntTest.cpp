#include <CXXSat/FlexInt.h>
#include <iostream>

int main() {
    FlexInt x{'c'};
    const FlexInt& y = x.cast(TypeInfo(true, 32));
    std::cout << x.getTypeInfo() << ' ' << y.getTypeInfo() << '\n';
    return 0;
}
