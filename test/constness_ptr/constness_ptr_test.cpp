#include "constnessptr.h"
#include "utils.h"

struct S
{
    void f();
    void f_c() const;
};

struct S1
{
    explicit S1(S*s) : mS(s) {}
    constness_ptr<S> mS;
};

void f()
{
    S s;
    S1 s1(&s);
    s1.mS->f();
    s1.mS->f_c();
    utils::as_const(s1).mS->f_c();
#ifdef BAD
    utils::as_const(s1).mS->f();
#endif
}
