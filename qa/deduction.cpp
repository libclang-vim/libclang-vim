#include <iostream>
#include <dlfcn.h>
#include <cppunit/extensions/HelperMacros.h>

class deduction_test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(deduction_test);
    CPPUNIT_TEST(test_current_function_at);
    CPPUNIT_TEST_SUITE_END();

    void test_current_function_at();

public:
    deduction_test();
    deduction_test(const deduction_test&) = delete;
    deduction_test& operator=(const deduction_test&) = delete;
};

deduction_test::deduction_test()
{
}

void deduction_test::test_current_function_at()
{
    void* handle = dlopen("lib/libclang-vim.so", RTLD_NOW);
    if (!handle)
    {
        std::stringstream ss;
        ss << "dlopen() failed: " ;
        ss << dlerror();
        CPPUNIT_FAIL(ss.str());
    }

    auto vim_clang_get_current_function_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(handle, "vim_clang_get_current_function_at"));
    CPPUNIT_ASSERT(vim_clang_get_current_function_at);

    std::string expected("{'name':'ns::C::foo'}");
    std::string actual(vim_clang_get_current_function_at("qa/data/current-function.cpp:-std=c++1y:13:1"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    dlclose(handle);
}

CPPUNIT_TEST_SUITE_REGISTRATION(deduction_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
