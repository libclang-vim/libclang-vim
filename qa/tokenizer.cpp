#include <iostream>
#include <dlfcn.h>
#include <unistd.h>
#include <cassert>
#include <cppunit/extensions/HelperMacros.h>

class tokenizer_test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(tokenizer_test);
    CPPUNIT_TEST(test_tokens);
    CPPUNIT_TEST(test_unsaved_tokens);
    CPPUNIT_TEST_SUITE_END();

    void test_tokens();
    void test_unsaved_tokens();

    void* m_handle;

public:
    tokenizer_test();
    tokenizer_test(const tokenizer_test&) = delete;
    tokenizer_test& operator=(const tokenizer_test&) = delete;

    void setUp() override;
    void tearDown() override;
};

tokenizer_test::tokenizer_test()
    : m_handle(nullptr)
{
}

void tokenizer_test::setUp()
{
    m_handle = dlopen("lib/libclang-vim.so", RTLD_NOW);
    if (!m_handle)
    {
        std::stringstream ss;
        ss << "dlopen() failed: " ;
        ss << dlerror();
        CPPUNIT_FAIL(ss.str());
    }
}

void tokenizer_test::tearDown()
{
    if (m_handle)
        dlclose(m_handle);
}

void tokenizer_test::test_tokens()
{
    auto vim_clang_tokens = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_tokens"));
    assert(vim_clang_tokens);

    // This resulted in a double free -> crash.
    std::string actual(vim_clang_tokens("qa/data/declaration.cpp:std=c++1y"));
    CPPUNIT_ASSERT(actual != "{}");
}

void tokenizer_test::test_unsaved_tokens()
{
    auto vim_clang_tokens = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_tokens"));
    assert(vim_clang_tokens);

    std::string actual(vim_clang_tokens("qa/data/unsaved/diagnostics.cpp#qa/data/unsaved/diagnostics-unsaved.cpp:std=c++1y"));
    // Unsaved file wasn't handled -> empty list was returned.
    CPPUNIT_ASSERT(actual != "[]");
}

CPPUNIT_TEST_SUITE_REGISTRATION(tokenizer_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
