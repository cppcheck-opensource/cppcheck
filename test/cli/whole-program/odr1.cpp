#include "odr.h"

#include <iostream>

// cppcheck-suppress ctuOneDefinitionRuleViolation
class C : public Base
{
public:
    void f() override {
        std::cout << "1";
    }
};

Base *c1_create()
{
    return new C();
}
