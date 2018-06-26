#include <cassert>
#include <cppunit/extensions/HelperMacros.h>
#include <dlfcn.h>
#include <iostream>
#include <unistd.h>

class deduction_test : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(deduction_test);
    // CPPUNIT_TEST(test_get_type_with_deduction_at);
    // CPPUNIT_TEST(test_unsaved_get_type_with_deduction_at);
    CPPUNIT_TEST(test_current_function_at);
    CPPUNIT_TEST(test_current_function_at_ctor_dtor);
    CPPUNIT_TEST(test_current_function_at_incomplete_type);
    CPPUNIT_TEST(test_unsaved_current_function_at);
    CPPUNIT_TEST(test_completion_at);
    CPPUNIT_TEST(test_unsaved_completion_at);
    CPPUNIT_TEST(test_comment_at);
    CPPUNIT_TEST(test_unsaved_comment_at);
    CPPUNIT_TEST(test_declaration_at);
    CPPUNIT_TEST(test_unsaved_declaration_at);
    CPPUNIT_TEST(test_compile_commands);
    CPPUNIT_TEST(test_include_at);
    CPPUNIT_TEST(test_unsaved_include_at);
    CPPUNIT_TEST(test_diagnostics);
    CPPUNIT_TEST(test_unsaved_diagnostics);
    CPPUNIT_TEST(test_full_name_at);
    CPPUNIT_TEST_SUITE_END();

    void test_get_type_with_deduction_at();
    void test_unsaved_get_type_with_deduction_at();
    void test_current_function_at();
    void test_current_function_at_ctor_dtor();
    void test_current_function_at_incomplete_type();
    void test_unsaved_current_function_at();
    void test_completion_at();
    void test_unsaved_completion_at();
    void test_comment_at();
    void test_unsaved_comment_at();
    void test_declaration_at();
    void test_unsaved_declaration_at();
    void test_compile_commands();
    void test_include_at();
    void test_unsaved_include_at();
    void test_diagnostics();
    void test_unsaved_diagnostics();
    void test_full_name_at();

    void* m_handle = nullptr;

  public:
    deduction_test();
    deduction_test(const deduction_test&) = delete;
    deduction_test& operator=(const deduction_test&) = delete;

    void setUp() override;
    void tearDown() override;
};

deduction_test::deduction_test() = default;

void deduction_test::setUp() {
    m_handle = dlopen("lib/libclang-vim.so", RTLD_NOW);
    if (!m_handle) {
        std::stringstream ss;
        ss << "dlopen() failed: ";
        ss << dlerror();
        CPPUNIT_FAIL(ss.str());
    }
}

void deduction_test::tearDown() {
    if (m_handle)
        dlclose(m_handle);
}

void deduction_test::test_get_type_with_deduction_at() {
    auto vim_clang_get_type_with_deduction_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_type_with_deduction_at"));
    assert(vim_clang_get_type_with_deduction_at);

#if (__clang_major__ == 3 && __clang_minor__ > 7) || __clang_major__ > 3
    std::string expected_prefix("{'type':'const std::basic_string<char> &'");
#else
    std::string expected_prefix("{'type':'const std::basic_string<char>'");
#endif
    std::string actual(vim_clang_get_type_with_deduction_at(
        "qa/data/auto.cpp:-std=c++1y:5:19"));
    CPPUNIT_ASSERT_EQUAL(
        0, actual.compare(0, expected_prefix.size(), expected_prefix));
}

void deduction_test::test_unsaved_get_type_with_deduction_at() {
    auto vim_clang_get_type_with_deduction_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_type_with_deduction_at"));
    assert(vim_clang_get_type_with_deduction_at);

#if (__clang_major__ == 3 && __clang_minor__ > 7) || __clang_major__ > 3
    std::string expected_prefix("{'type':'const std::basic_string<char> &'");
#else
    std::string expected_prefix("{'type':'const std::basic_string<char>'");
#endif
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_type_with_deduction_at(
        "auto.cpp#../auto.cpp:-std=c++1y:5:19"));
    chdir("../../..");
    CPPUNIT_ASSERT_EQUAL(
        0, actual.compare(0, expected_prefix.size(), expected_prefix));
}

void deduction_test::test_current_function_at() {
    auto vim_clang_get_current_function_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_current_function_at"));
    assert(vim_clang_get_current_function_at);

    std::string expected("{'name':'ns::C::foo'}");
    std::string actual(vim_clang_get_current_function_at(
        "qa/data/current-function.cpp:-std=c++1y:10:1"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_unsaved_current_function_at() {
    auto vim_clang_get_current_function_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_current_function_at"));
    assert(vim_clang_get_current_function_at);

    std::string expected("{'name':'ns::C::foo'}");
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_current_function_at(
        "current-function.cpp#../current-function.cpp:-std=c++1y:10:1"));
    chdir("../../..");
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_current_function_at_ctor_dtor() {
    auto vim_clang_get_current_function_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_current_function_at"));
    assert(vim_clang_get_current_function_at);

    std::string expected("{'name':'D::D'}");
    std::string actual(vim_clang_get_current_function_at(
        "qa/data/current-function.cpp:-std=c++1y:20:9"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    expected = ("{'name':'D::~D'}");
    actual = vim_clang_get_current_function_at(
        "qa/data/current-function.cpp:-std=c++1y:22:11");
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_current_function_at_incomplete_type() {
    auto vim_clang_get_current_function_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_current_function_at"));
    assert(vim_clang_get_current_function_at);

    std::string expected("{'name':'func'}");
    std::string actual(vim_clang_get_current_function_at(
        "qa/data/current-function.cpp:-std=c++1y:26:24"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_completion_at() {
    auto vim_clang_get_completion_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_completion_at"));
    assert(vim_clang_get_completion_at);

    std::string expected("['C', 'bar', 'foo', 'operator=', '~C']");
    std::string actual(
        vim_clang_get_completion_at("qa/data/completion.cpp:-std=c++1y:16:7"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_unsaved_completion_at() {
    auto vim_clang_get_completion_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_completion_at"));
    assert(vim_clang_get_completion_at);

    std::string expected("['C', 'bar', 'foo', 'operator=', '~C']");
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_completion_at(
        "completion.cpp#completion-unsaved.cpp:-std=c++1y:16:7"));
    chdir("../../..");
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_comment_at() {
    auto vim_clang_get_completion_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_comment_at"));
    assert(vim_clang_get_completion_at);

    std::string expected("{'brief':'This is foo.'}");
    std::string actual(vim_clang_get_completion_at(
        "qa/data/current-function.cpp:-std=c++1y:37:8"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_unsaved_comment_at() {
    auto vim_clang_get_comment_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_comment_at"));
    assert(vim_clang_get_comment_at);

    std::string expected("{'brief':'This is foo.'}");
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_comment_at(
        "current-function.cpp#../current-function.cpp:-std=c++1y:37:8"));
    chdir("../../..");
    // This was "{}", unsaved file support was missing.
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_declaration_at() {
    auto vim_clang_get_deduced_declaration_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_deduced_declaration_at"));
    assert(vim_clang_get_deduced_declaration_at);

    std::string expected(
        "{'file':'qa/data/declaration.cpp','line':'3','col':'7',}");
    std::string actual(vim_clang_get_deduced_declaration_at(
        "qa/data/declaration.cpp:-std=c++1y:3:15"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_unsaved_declaration_at() {
    auto vim_clang_get_deduced_declaration_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_deduced_declaration_at"));
    assert(vim_clang_get_deduced_declaration_at);

    std::string expected("{'file':'./declaration.hpp','line':'1','col':'5',}");
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_deduced_declaration_at(
        "declaration.cpp#declaration-unsaved.cpp:-std=c++1y:3:15"));
    chdir("../../..");
    // This was "{}", relative include broke without unsaved file support.
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_compile_commands() {
    auto vim_clang_get_compile_commands =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_compile_commands"));
    assert(vim_clang_get_compile_commands);

    std::string expected("{'commands':'clang++ -DFOO -I" SRC_ROOT
                         "/qa/data/compile-commands -o test.o -c'}");
    std::string actual(vim_clang_get_compile_commands(
        SRC_ROOT "/qa/data/compile-commands/test.cpp:"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_include_at() {
    auto vim_clang_get_include_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_include_at"));
    assert(vim_clang_get_include_at);

    std::string expected("{'file':'" SRC_ROOT
                         "/qa/data/compile-commands/test.hpp'}");
    std::string actual(vim_clang_get_include_at(
        "qa/data/compile-commands/test.cpp:-std=c++1y -I" SRC_ROOT
        "/qa/data/compile-commands/:1:2"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_unsaved_include_at() {
    auto vim_clang_get_include_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_include_at"));
    assert(vim_clang_get_include_at);

    std::string expected("{'file':'./include.hpp'}");
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_include_at(
        "include.cpp#include-unsaved.cpp:-std=c++1y:1:14"));
    chdir("../../..");
    // This was "{}", relative include broke without unsaved file support.
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_diagnostics() {
    auto vim_clang_get_diagnostics =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_diagnostics"));
    assert(vim_clang_get_diagnostics);

    std::string expected("[{'severity': 'warning', "
                         "'line':1,'column':18,'offset':17,'file':'qa/data/"
                         "diagnostics.cpp',}, ]");
    std::string actual(
        vim_clang_get_diagnostics("qa/data/diagnostics.cpp:-Wunused-variable"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_unsaved_diagnostics() {
    auto vim_clang_get_diagnostics =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_diagnostics"));
    assert(vim_clang_get_diagnostics);

    std::string expected("[{'severity': 'warning', "
                         "'line':1,'column':18,'offset':17,'file':'diagnostics."
                         "cpp',}, ]");
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_diagnostics(
        "diagnostics.cpp#diagnostics-unsaved.cpp:-Wunused-variable"));
    chdir("../../..");
    // This was "[]", unsaved file support was missing.
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void deduction_test::test_full_name_at() {
    auto vim_clang_get_full_name_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_full_name_at"));
    assert(vim_clang_get_full_name_at);

    std::string expected("{'name':'E::foo'}");
    std::string actual(vim_clang_get_full_name_at(
        "qa/data/current-function.cpp:-std=c++1y:37:8"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

CPPUNIT_TEST_SUITE_REGISTRATION(deduction_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
