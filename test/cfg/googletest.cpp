
// Test library configuration for googletest.cfg
//
// Usage:
// $ cppcheck --check-library --library=googletest --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/googletest.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file valueFlowBailout

#include <gmock/gmock-generated-matchers.h>
#include <gtest/gtest.h>


namespace ExampleNamespace {
    constexpr long long TOLERANCE = 10;

    // #9397 syntaxError when MATCHER_P is not known
    // cppcheck-suppress symbolDatabaseWarning
    MATCHER_P(ExampleMatcherPTest, expected, "")
    {
        // cppcheck-suppress valueFlowBailoutIncompleteVar
        return ((arg <= (expected + TOLERANCE)) && (arg >= (expected - TOLERANCE)));
    }

    // syntaxError when MATCHER is not known
    // cppcheck-suppress symbolDatabaseWarning
    MATCHER(ExampleMatcherTest, "")
    {
        return (arg == TOLERANCE);
    }

    // syntaxError when TYPED_TEST is not known
    TYPED_TEST (ExampleTypedTest, cppcheck_test)
    {
    }

    // syntaxError when TYPED_TEST_P is not known
    TYPED_TEST_P (ExampleTypedTestP, cppcheck)
    {
    }
}

TEST(ASSERT, ASSERT)
{
    int *a = (int*)calloc(10,sizeof(int)); // cppcheck-suppress cstyleCast
    ASSERT_TRUE(a != nullptr);

    a[0] = 10;

    free(a);
}

// Avoid syntax error: https://sourceforge.net/p/cppcheck/discussion/general/thread/6ccc7283e2/
TEST(test_cppcheck, cppcheck)
{
    TestStruct<int> it;
    ASSERT_THROW(it.operator->(), std::out_of_range);
}

// #9964 - avoid compareBoolExpressionWithInt false positive
TEST(Test, assert_false_fp)
{
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    ASSERT_FALSE(errno < 0);
}

// Check that conditions in the ASSERT_* macros are processed correctly.
TEST(Test, warning_in_assert_macros)
{
    int i = 5;
    int j = 6;

    // cppcheck-suppress knownConditionTrueFalse
    ASSERT_TRUE(i == 5);
    // cppcheck-suppress knownConditionTrueFalse
    ASSERT_FALSE(i != 5);
    // cppcheck-suppress duplicateExpression
    ASSERT_EQ(i, i);
    ASSERT_NE(i, j); // expected knownConditionTrueFalse...
    ASSERT_LT(i, j); // expected knownConditionTrueFalse...
    // cppcheck-suppress duplicateExpression
    ASSERT_LE(i, i);
    ASSERT_GT(j, i); // expected knownConditionTrueFalse
    // cppcheck-suppress duplicateExpression
    ASSERT_GE(i, i);

    // cppcheck-suppress valueFlowBailoutIncompleteVar
    unsigned int u = errno;
    // cppcheck-suppress [unsignedPositive]
    ASSERT_GE(u, 0);
    // cppcheck-suppress [unsignedLessThanZero]
    ASSERT_LT(u, 0);
}
