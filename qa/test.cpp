#include <iostream>
#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/CompilerOutputter.h>

int main() {
    CPPUNIT_NS::TestResult controller;
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);

    CPPUNIT_NS::TestRunner runner;

    const char* test_name = getenv("CPPUNIT_TEST_NAME");
    if (test_name) {
        // E.g. 'make check CPPUNIT_TEST_NAME="tokenizer_test::test_tokens"',
        // single test.
        CppUnit::Test* test_registry =
            CppUnit::TestFactoryRegistry::getRegistry().makeTest();
        for (int i = 0; i < test_registry->getChildTestCount(); ++i) {
            CppUnit::Test* test_suite = test_registry->getChildTestAt(i);
            for (int j = 0; j < test_suite->getChildTestCount(); ++j) {
                CppUnit::Test* test_case = test_suite->getChildTestAt(j);
                if (test_case->getName() == test_name)
                    runner.addTest(test_case);
            }
        }
    } else
        // 'make check', all tests.
        runner.addTest(
            CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());

    runner.run(controller);

    CPPUNIT_NS::CompilerOutputter outputter(&result, std::cerr);
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
