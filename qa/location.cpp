#include <iostream>
#include <dlfcn.h>
#include <unistd.h>
#include <cassert>
#include <cppunit/extensions/HelperMacros.h>

class location_test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(location_test);
    CPPUNIT_TEST(test_all_extents);
    CPPUNIT_TEST(test_unsaved_all_extents);
    CPPUNIT_TEST_SUITE_END();

    void test_all_extents();
    void test_unsaved_all_extents();

    void* m_handle;

public:
    location_test();
    location_test(const location_test&) = delete;
    location_test& operator=(const location_test&) = delete;

    void setUp() override;
    void tearDown() override;
};

location_test::location_test()
    : m_handle(nullptr)
{
}

void location_test::setUp()
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

void location_test::tearDown()
{
    if (m_handle)
        dlclose(m_handle);
}

void location_test::test_all_extents()
{
    auto vim_clang_get_all_extents_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_all_extents_at"));
    assert(vim_clang_get_all_extents_at);

    std::string expected_prefix = "[{'start':{'line':2,'column':1,'offset':12,'file':'qa/data/all-extents.cpp',},'end':{'line':5,'column':2,'offset':37,'file':'qa/data/all-extents.cpp',}}";
    std::string actual(vim_clang_get_all_extents_at("qa/data/all-extents.cpp:std=c++1y:3:1"));
    CPPUNIT_ASSERT_EQUAL(0, actual.compare(0, expected_prefix.size(), expected_prefix));
}

void location_test::test_unsaved_all_extents()
{
    auto vim_clang_get_all_extents_at = reinterpret_cast<char const* (*)(char const*)>(dlsym(m_handle, "vim_clang_get_all_extents_at"));
    assert(vim_clang_get_all_extents_at);

    std::string expected_prefix = "[{'start':{'line':2,'column':1,'offset':12,'file':'all-extents.cpp',},'end':{'line':5,'column':2,'offset':37,'file':'all-extents.cpp',}}";
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_all_extents_at("all-extents.cpp#../all-extents.cpp:-std=c++1y:3:1"));
    chdir("../../..");
    CPPUNIT_ASSERT_EQUAL(0, actual.compare(0, expected_prefix.size(), expected_prefix));
}

CPPUNIT_TEST_SUITE_REGISTRATION(location_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
