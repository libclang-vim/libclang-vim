#include <string>

std::string f() { return "x"; }

void g() { const auto& s = f(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
