struct Dummy {
 int x;
};
void func() {
  // cppcheck-suppress threadsafety-threadsafety-const
  static const Dummy dummy;
}
