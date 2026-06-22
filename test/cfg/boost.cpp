
// Test library configuration for boost.cfg
//
// Usage:
// $ cppcheck --check-library --library=boost --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/boost.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file valueFlowBailout

#include <cstdio>
#include <new>
#include <tuple>

#include <boost/config.hpp> // IWYU pragma: keep
#include <boost/math/special_functions/round.hpp>
#include <boost/endian/conversion.hpp> // IWYU pragma: keep
#include <boost/bind/bind.hpp>
#include <boost/function.hpp> // IWYU pragma: keep
#include <boost/smart_ptr/scoped_array.hpp>
#include <boost/thread/mutex.hpp> // IWYU pragma: keep
#include <boost/thread/lock_guard.hpp>
#include <boost/test/unit_test.hpp> // IWYU pragma: keep
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/core/scoped_enum.hpp>
#include <boost/foreach.hpp>

#include <cstdlib>
#include <set>
#include <vector>

BOOST_FORCEINLINE void boost_forceinline_test()
{}

BOOST_NOINLINE void boost_noinline_test()
{}

BOOST_NORETURN void boost_noreturn_test()
{}

void print_hello()
{
    printf("hello");
}

void valid_code(boost::function<void(void)> &pf_print_hello)
{
    if (BOOST_LIKELY(1)) {}
    if (BOOST_UNLIKELY(0)) {}

    int int1 = 5;
    boost::endian::endian_reverse_inplace(int1);
    boost::bind(print_hello)();
    pf_print_hello = boost::bind(print_hello);
}

void ignoredReturnValue()
{
    // cppcheck-suppress ignoredReturnValue
    boost::math::round(1.5);
    // cppcheck-suppress ignoredReturnValue
    boost::math::iround(1.5);
    // cppcheck-suppress ignoredReturnValue
    boost::math::lround(1.5);
    // cppcheck-suppress ignoredReturnValue
    boost::math::llround(1.5);
    // cppcheck-suppress ignoredReturnValue
    boost::endian::endian_reverse(1);
}

void uninitvar()
{
    int intUninit1;
    int intUninit2;
    // cppcheck-suppress uninitvar
    boost::endian::endian_reverse_inplace(intUninit1);
    // cppcheck-suppress uninitvar
    (void)boost::math::round(intUninit2);
}

void throwexception(int * buf)
{
    if (!buf)
        boost::throw_exception(std::bad_alloc());
    *buf = 0;
}

void throwexception2(int * buf)
{
    if (!buf)
        BOOST_THROW_EXCEPTION(std::bad_alloc());
    *buf = 0;
}

void macros()
{
#define DECL(z, n, text) text ## n = n;
    BOOST_PP_REPEAT(5, DECL, int x)

    BOOST_SCOPED_ENUM_DECLARE_BEGIN(future_errc) {
        // cppcheck-suppress valueFlowBailoutIncompleteVar
        no_state
    }
    BOOST_SCOPED_ENUM_DECLARE_END(future_errc)
}

void containerOutOfBounds_scoped_array(std::size_t n) // #12356
{
    boost::scoped_array<int> d(new int[n] {});
    for (std::size_t i = 0; i < n; i++)
        if (d[i]) {}
}

void lock_guard_finiteLifetime(boost::mutex& m)
{
    // cppcheck-suppress unusedScopedObject
    boost::lock_guard<boost::mutex>{ m };
}

void test_BOOST_FOREACH_1(std::vector<int> data)
{
    BOOST_FOREACH(int i, data) {
        // cppcheck-suppress invalidContainerLoop
        data.push_back(123);
    }
}

void test_BOOST_FOREACH_2(std::set<int> data)
{
    BOOST_FOREACH(int i, data) {
        // don't warn for std::set
        data.insert(123);
    }
}

void test_BOOST_FOREACH_3(std::vector<int> data)
{
    BOOST_FOREACH(const int& i, data) {
        // cppcheck-suppress invalidContainerLoop
        data.erase(data.begin());
    }
}

// Check single line usage
void test_BOOST_FOREACH_4(std::vector<int> data)
{
    BOOST_FOREACH(const int& i, data)
        // cppcheck-suppress invalidContainerLoop
        data.clear();
}

// Container returned as result of a function -> Be quiet
std::vector<int> get_data();
void test_BOOST_FOREACH_5()
{
    std::set<int> data;
    BOOST_FOREACH(const int& i, get_data())
        // cppcheck-suppress useStlAlgorithm
        data.insert(i);
}

// Break after modification (#4788)
void test_BOOST_FOREACH_6(std::vector<int> data)
{
    BOOST_FOREACH(int i, data) {
        data.push_back(123);
        break;
    }
}

void test_require()
{
    int *some_int = static_cast<int*>(std::malloc(sizeof(int)));
    int *some_other_int = static_cast<int*>(malloc(sizeof(int)));
    BOOST_REQUIRE(some_int);
    BOOST_TEST_REQUIRE(some_other_int);
    *some_int = 42;
    *some_other_int = 42;
    std::free(some_int);
    free(some_other_int);
}

BOOST_AUTO_TEST_SUITE(my_auto_test_suite)

BOOST_AUTO_TEST_CASE(test_message_macros)
{
    bool my_bool = false;
    BOOST_WARN_MESSAGE(my_bool, "warn");
    BOOST_CHECK_MESSAGE(my_bool, "check");
    BOOST_REQUIRE_MESSAGE(my_bool, "require");

    BOOST_WARN_MESSAGE(my_bool, "my_bool was: " << my_bool);
}

using test_types_w_tuples = std::tuple<int, long, unsigned char>;
BOOST_AUTO_TEST_CASE_TEMPLATE(my_tuple_test, T, test_types_w_tuples)
{
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    BOOST_TEST(sizeof(T) == 4U);
}

BOOST_AUTO_TEST_SUITE_END()

// https://www.boost.org/doc/libs/latest/libs/test/doc/html/boost_test/tests_organization/test_cases/test_case_generation/datasets.html
namespace bdata = boost::unit_test::data;

// Dataset generating a Fibonacci sequence
struct fibonacci_dataset {
    // the type of the samples is deduced
    // cppcheck-suppress unusedStructMember // FP #14795, used in template is_dataset
    static const int arity = 1;

    struct iterator {
        int operator*() const { return b; }
        void operator++() {
            a = a + b;
            std::swap(a, b);
        }
    private:
        int a = 1;
        int b = 1; // b is the output
    };

    // size is infinite
    bdata::size_t size() const { return bdata::BOOST_TEST_DS_INFINITE_SIZE; }

    // iterator
    static iterator begin() { return iterator(); }
};

namespace boost { namespace unit_test { namespace data { namespace monomorphic {
  // registering fibonacci_dataset as a proper dataset
  template <>
  struct is_dataset<fibonacci_dataset> : boost::mpl::true_ {};
}}}}

// Creating a test-driven dataset, the zip is for checking
BOOST_DATA_TEST_CASE(
    test1,
    fibonacci_dataset() ^ bdata::make( { 1, 2, 3, 5, 8, 13, 21, 35, 56 } ),
    fib_sample, exp)
{
      // cppcheck-suppress valueFlowBailoutIncompleteVar // TODO: fib_sample declared in test case
      BOOST_TEST(fib_sample == exp);
}
