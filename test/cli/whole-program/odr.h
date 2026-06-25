class Base {
public:
    virtual void f() = 0;
};

extern Base *c1_create();
extern Base *c2_create();