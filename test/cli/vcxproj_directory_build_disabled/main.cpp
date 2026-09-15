// Each check below fails with #error if Directory.Build.props/.targets was
// imported despite ImportDirectoryBuildProps/ImportDirectoryBuildTargets being
// set to false; see vcxproj_directory_build_disabled_test.py.
#if defined(PROPS_DEFINE)
#error Directory.Build.props was imported despite ImportDirectoryBuildProps=false
#endif
#if defined(TARGETS_DEFINE)
#error Directory.Build.targets was imported despite ImportDirectoryBuildTargets=false
#endif

int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
