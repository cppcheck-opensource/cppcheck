// To test:
// ~/cppcheck/cppcheck --dump naming_test.c && python ../naming.py --var='[a-z].*' --function='[a-z].*' naming_test.c.dump

// Should not crash when there is no name
void func(int number, int);
