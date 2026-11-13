// Reached (division-by-zero branch) only if BOTH BASE_DEFINE and
// EXTRA_DEFINE ended up defined -- which requires addMetadata()'s
// case-mismatched "$(preprocessordefinitions)" self-reference (see the
// second ItemDefinitionGroup in the .vcxproj) to have correctly resolved to
// "BASE_DEFINE", accumulated from the first ItemDefinitionGroup. Before the
// fix, that self-reference was left as literal, unresolved macro text
// instead -- so BASE_DEFINE was never actually defined, and this file would
// take the "not defined" branch below instead, hiding the division by zero.
// See vcxproj_metadata_self_reference_case_test.py.
#if defined(BASE_DEFINE) && defined(EXTRA_DEFINE)
int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
#else
int main()
{
    return 0;
}
#endif
