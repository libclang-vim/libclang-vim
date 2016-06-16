namespace ns {

class C {
  public:
    void foo(const char*);
    void foo(int);
};
}

int main() {
    ns::C c;
    c.foo("foo");
    c.foo(1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
