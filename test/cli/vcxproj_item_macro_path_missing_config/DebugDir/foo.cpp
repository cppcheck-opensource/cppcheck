#error DebugDir/foo.cpp must never be checked -- $(MyRoot) is unset in Release|x64, so it genuinely varies by configuration and must not be expanded
int main() { return 0; }
