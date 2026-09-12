// Each check below fails with #error unless the wildcard <Import Project=
// "ImportBefore\*.props" /> correctly expanded to import BOTH a.props and
// b.props (sorted, per MSBuild's own documented behavior), while skipping
// decoy.txt (extension doesn't match "*.props") and the nonexistent
// ImportAfter\*.props wildcard resolved to nothing without error. See
// vcxproj_import_wildcard_test.py.
#if !defined(WILDCARD_A_DEFINE)
#error ImportBefore/a.props (matched by wildcard Import) was not imported
#endif
#if !defined(WILDCARD_B_DEFINE)
#error ImportBefore/b.props (matched by wildcard Import) was not imported
#endif

int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
