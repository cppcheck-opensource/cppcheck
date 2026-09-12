// Each check below fails with #error if DirectoryBuildPropsPath/
// DirectoryBuildTargetsPath did not override the upward directory search with
// the explicit file they name; see vcxproj_directory_build_path_test.py.
#if !defined(RIGHT_PROPS_DEFINE)
#error DirectoryBuildPropsPath override (CustomBuild.props) was not imported
#endif
#if defined(WRONG_PROPS_DEFINE)
#error Directory.Build.props was imported even though DirectoryBuildPropsPath overrides the search
#endif
#if !defined(RIGHT_TARGETS_DEFINE)
#error DirectoryBuildTargetsPath override (CustomBuild.targets) was not imported
#endif
#if defined(WRONG_TARGETS_DEFINE)
#error Directory.Build.targets was imported even though DirectoryBuildTargetsPath overrides the search
#endif

int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
