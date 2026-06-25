
// Test library configuration for libsigc++.cfg
//
// Usage:
// $ cppcheck --check-library --library=libsigc++ --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/libsigc++.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <sigc++/sigc++.h> // IWYU pragma: keep
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/slot.h>
#include <sigc++/trackable.h>

struct struct1 : public sigc::trackable {
    void func1(int) const {}
};

void valid_code()
{
    const struct1 my_sruct1;
    sigc::slot<void, int> sl = sigc::mem_fun(my_sruct1, &struct1::func1);
    if (sl) {}
}

void ignoredReturnValue()
{
    const struct1 my_sruct1;
    // cppcheck-suppress ignoredReturnValue
    sigc::mem_fun(my_sruct1, &struct1::func1);
}
