struct Dummy {
 int x;
};
void func() {
    // cppcheck-suppress threadsafety-threadsafety
    static Dummy dummy;
}
