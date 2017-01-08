#include <cassert>
#include <cppunit/extensions/HelperMacros.h>
#include <dlfcn.h>
#include <iostream>
#include <unistd.h>

class ast_test : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(ast_test);
    CPPUNIT_TEST(test_extract_declarations_current_file);
    CPPUNIT_TEST(test_unsaved_extract_declarations_current_file);
    CPPUNIT_TEST_SUITE_END();

    void test_extract_declarations_current_file();
    void test_unsaved_extract_declarations_current_file();

    void* m_handle = nullptr;

  public:
    ast_test();
    ast_test(const ast_test&) = delete;
    ast_test& operator=(const ast_test&) = delete;

    void setUp() override;
    void tearDown() override;
};

ast_test::ast_test() = default;

void ast_test::setUp() {
    m_handle = dlopen("lib/libclang-vim.so", RTLD_NOW);
    if (!m_handle) {
        std::stringstream ss;
        ss << "dlopen() failed: ";
        ss << dlerror();
        CPPUNIT_FAIL(ss.str());
    }
}

void ast_test::tearDown() {
    if (m_handle)
        dlclose(m_handle);
}

void ast_test::test_extract_declarations_current_file() {
    auto vim_clang_extract_declarations_current_file =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_extract_declarations_current_file"));
    assert(vim_clang_extract_declarations_current_file);

    std::string actual(vim_clang_extract_declarations_current_file(
        "qa/data/declaration.cpp#:std=c++1y"));
    CPPUNIT_ASSERT(actual != "{}");
}

void ast_test::test_unsaved_extract_declarations_current_file() {
    auto vim_clang_extract_declarations_current_file =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_extract_declarations_current_file"));
    assert(vim_clang_extract_declarations_current_file);

    std::string actual(vim_clang_extract_declarations_current_file(
        "qa/data/unsaved/auto.cpp#qa/data/declaration.cpp:std=c++1y"));
    CPPUNIT_ASSERT(actual != "{'root':[]}");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ast_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
