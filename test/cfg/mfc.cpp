
// Test library configuration for mfc.cfg

#include <afxwin.h>


class MyClass1 : public CObject {
    DECLARE_DYNAMIC(MyClass1)
public:
    MyClass1() {}
};
IMPLEMENT_DYNAMIC(MyClass1, CObject)

class MyClass2 : public CObject {
    DECLARE_DYNCREATE(MyClass2)
public:
    MyClass2() {}
};
IMPLEMENT_DYNCREATE(MyClass2, CObject)

class MyClass3 : public CObject {
    DECLARE_SERIAL(MyClass3)
public:
    MyClass3() {}
};
IMPLEMENT_SERIAL(MyClass3, CObject, 42)
