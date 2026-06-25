// Test library configuration for cppunit.cfg
//
// Usage:
// $ cppcheck --check-library --library=cppunit --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/cppunit.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file valueFlowBailout

#include <string>

#include <cppunit/Exception.h>
#include <cppunit/Portability.h>
#include <cppunit/TestAssert.h>

void cppunit_assert_equal(int x, double y)
{
    CPPUNIT_ASSERT(true);
    CPPUNIT_ASSERT(false);
    CPPUNIT_ASSERT(1 < 2);

    CPPUNIT_ASSERT_MESSAGE("Test failed", true);
    CPPUNIT_ASSERT_MESSAGE("Test failed", false);
    CPPUNIT_ASSERT_MESSAGE("Test failed", 1 < 2);

    CPPUNIT_ASSERT_EQUAL(1, 2);
    CPPUNIT_ASSERT_EQUAL(true, 3 == x);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Test failed", 1, 4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Test failed", true, 4 == x);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, 2.0, 1e-7);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, y, 1e-7);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Test failed", 1.0, 2.0, 1e-7);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Test failed", 1.0, y, 1e-7);
}

void cppunit_throw()
{
    CPPUNIT_ASSERT_NO_THROW(1 + 1);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unexpected throw", 1 + 1);
    CPPUNIT_ASSERT_THROW(1 + 1, CPPUNIT_NS::Exception);
    CPPUNIT_ASSERT_THROW_MESSAGE("Did not throw", 1 + 1, CPPUNIT_NS::Exception);
}

void cppunit_assertion_assert()
{
    CPPUNIT_ASSERT_ASSERTION_FAIL(true);
    CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("hello", false);
    CPPUNIT_ASSERT_ASSERTION_PASS(false);
    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE("goodbye", true);
}

void cppunit_assert_fail()
{
    CPPUNIT_FAIL("This fails");
}
