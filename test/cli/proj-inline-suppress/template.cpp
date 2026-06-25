namespace {
    template<typename T>
    T f() {
        return T();
    }
}

// cppcheck-suppress unusedFunction
int g(int i) {
    if (i == 0)
        return f<char>();
    if (i == 1)
        return f<short>();
    return f<int>();
}