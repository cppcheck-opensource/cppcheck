// Each check below fails with #error when the import graph is not walked the way
// MSBuild / Visual Studio walk it; see vcxproj_import_graph_test.py.
#if !defined(GUARDED_DEFINE)
#error ItemDefinitionGroup of guarded.props (import guard) was not applied
#endif
#if DUP_IMPORT_COUNT != 1
#error dup.props was imported more than once
#endif
#if !defined(DIRBUILD_DEFINE)
#error ItemDefinitionGroup of Directory.Build.props was not applied
#endif
#if !defined(PROJ_DEFINE)
#error ItemDefinitionGroup of the project was not applied
#endif

int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
