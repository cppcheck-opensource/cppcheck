#error ReleaseCaseDir/foo.cpp must never be checked -- $(CaseRoot) genuinely varies by configuration (case-insensitively) and must not be expanded
int main() { return 0; }
