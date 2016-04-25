#include <iostream>
#include <dlfcn.h>
#include <cppunit/extensions/HelperMacros.h>

class deduction_test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(deduction_test);
    CPPUNIT_TEST(test_current_function_at);
    CPPUNIT_TEST(test_current_function_at_ctor_dtor);
    CPPUNIT_TEST(test_current_function_at_incomplete_type);
    CPPUNIT_TEST(test_completion_at);
    CPPUNIT_TEST(test_comment_at);
    CPPUNIT_TEST(test_declaration_at);
    CPPUNIT_TEST(test_compile_commands);
    CPPUNIT_TEST(test_include_at);
    CPPUNIT_TEST_SUITE_END();

    void test_current_function_at();
    void test_current_function_at_ctor_dtor();
    void test_current_function_at_incomplete_type();
    void test_completion_at();
    void test_comment_at();
    void test_declaration_at();
    void test_compile_commands();
    void test_include_at();

    void* m_handle;

public:
    deduction_test();
    deduction_test(const deduction_test&) = delete;
    deduction_test& operator=(const deduction_test&) = delete;

    virtual void setUp() override;
    virtual void tearDown() override;
};

deduction_test::deduction_test()
    : m_handle(nullptr)
{
}

void deduction_test::setUp()
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

void deduction_test::tearDown()
{
    if (m_handle)
        dlclose(m_handle);
}

void deduction_test::test_current_function_at()
{
    auto vim_clang_get_current_function_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_current_function_at"));
    CPPUNIT_ASSERT(vim_clang_get_current_function_at);

    std::string expected("{'name':'ns::C::foo'}");
    std::string actual(vim_clang_get_current_function_at("qa/data/current-function.cpp:-std=c++1y:13:1"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_current_function_at_ctor_dtor()
{
    auto vim_clang_get_current_function_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_current_function_at"));
    CPPUNIT_ASSERT(vim_clang_get_current_function_at);

    std::string expected("{'name':'D::D'}");
    std::string actual(vim_clang_get_current_function_at("qa/data/current-function.cpp:-std=c++1y:26:1"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    expected = ("{'name':'D::~D'}");
    actual = vim_clang_get_current_function_at("qa/data/current-function.cpp:-std=c++1y:31:1");
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_current_function_at_incomplete_type()
{
    auto vim_clang_get_current_function_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_current_function_at"));
    CPPUNIT_ASSERT(vim_clang_get_current_function_at);

    std::string expected("{'name':'func'}");
    std::string actual(vim_clang_get_current_function_at("qa/data/current-function.cpp:-std=c++1y:38:7"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_completion_at()
{
    auto vim_clang_get_completion_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_completion_at"));
    CPPUNIT_ASSERT(vim_clang_get_completion_at);

    std::string expected("['C', 'bar', 'foo', 'operator=', '~C']");
    std::string actual(vim_clang_get_completion_at("qa/data/completion.cpp:-std=c++1y:25:7"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_comment_at()
{
    auto vim_clang_get_completion_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_comment_at"));
    CPPUNIT_ASSERT(vim_clang_get_completion_at);

    std::string expected("{'brief':'This is foo.'}");
    std::string actual(vim_clang_get_completion_at("qa/data/current-function.cpp:-std=c++1y:54:8"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_declaration_at()
{
    auto vim_clang_get_deduced_declaration_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_deduced_declaration_at"));
    CPPUNIT_ASSERT(vim_clang_get_deduced_declaration_at);

    std::string expected("{'file':'qa/data/declaration.cpp','line':'7','col':'10',}");
    std::string actual(vim_clang_get_deduced_declaration_at("qa/data/declaration.cpp:-std=c++1y:17:8"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_compile_commands()
{
    auto vim_clang_get_compile_commands = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_compile_commands"));
    CPPUNIT_ASSERT(vim_clang_get_compile_commands);

    std::string expected("{'commands':'clang++ -DFOO -I" SRC_ROOT "/qa/data/compile-commands -o test.o -c'}");
    std::stringstream ss;
    ss << getenv("PWD");
    ss << "/qa/data/compile-commands/test.cpp:";
    std::string actual(vim_clang_get_compile_commands(ss.str().c_str()));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_include_at()
{
    auto vim_clang_get_include_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_include_at"));
    CPPUNIT_ASSERT(vim_clang_get_include_at);

    std::string expected("{'file':'" SRC_ROOT "/qa/data/compile-commands/test.hpp'}");
    std::string actual(vim_clang_get_include_at("qa/data/compile-commands/test.cpp:-std=c++1y -I" SRC_ROOT "/qa/data/compile-commands/:1:2"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

CPPUNIT_TEST_SUITE_REGISTRATION(deduction_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
