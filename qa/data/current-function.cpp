#include <iostream>

namespace ns {

class C {
    int foo(int x);
};

int C::foo(int x) {
    int y = 0;
    return y;
}
}

class D {
    D();
    ~D();
};

D::D() { int x; }

D::~D() { int x; }

class Buffer;

static void func() { Buffer* buf; }

struct E {
    /// This is foo.
    void foo();
};

void E::foo() {}

static void func2() {
    E e;
    e.foo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
