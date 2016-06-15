#include <cassert>
#include <dlfcn.h>

#include <iostream>

/// Can invoke functions from cmdline the same way as Vimscript does it via
/// libcall(), to help debugging.
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <name> <input>" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Example: " << argv[0]
                  << " vim_clang_get_current_function_at "
                     "'qa/data/current-function.cpp:-std=c++1y:13:1'"
                  << std::endl;
        return 1;
    }
    const char* name = argv[1];
    const char* input = argv[2];

    void* handle = dlopen(SRC_ROOT "/lib/libclang-vim.so", RTLD_NOW);
    if (!handle) {
        std::cerr << "dlopen() failed: " << dlerror() << std::endl;
        return 1;
    }

    auto function =
        reinterpret_cast<char const* (*)(char const*)>(dlsym(handle, name));
    assert(function);

    std::cout << "Output is: '" << function(input) << "'." << std::endl;

    dlclose(handle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
