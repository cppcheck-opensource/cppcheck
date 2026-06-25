// To test:
// ~/cppcheck/cppcheck --dump naming_test.cpp && python ../naming.py --var='[a-z].*' --function='[a-z].*' naming_test.cpp.dump

// No error for mismatching Constructor/Destructor names should be issued, they can not be changed.
class TestClass1
{
    TestClass1() {}
    ~TestClass1() {}
    TestClass1(const TestClass1 &) {}
    TestClass1(TestClass1 &&) {}
};
