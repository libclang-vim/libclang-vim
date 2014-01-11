#include <clang-c/Index.h>

#ifdef __cplusplus
extern "C" {
#endif

char const*vim_clang_version()
{
    CXString version = clang_getClangVersion();
    return clang_getCString(version);
}

#ifdef __cplusplus
} // extern "C"
#endif

#include <iostream>

int main()
{
    std::cout << vim_clang_version() << '\n';
    return 0;
}
