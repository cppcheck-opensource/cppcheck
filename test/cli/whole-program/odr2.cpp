#include "odr.h"

#include <iostream>

class C : public Base
{
public:
    void f() override {
        std::cout << "2";
    }
};

Base *c2_create()
{
    return new C();
}
