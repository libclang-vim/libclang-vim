#include <iostream>

namespace ns {
class C {
    int foo(int x);
    int bar(int x);
};

int C::foo(int x) { int y = 0; }
int C::bar(int x) { int y = 0; }
}

int main() {
    std::cout << "hello" << std::endl;
    ns::C c;
    c.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
