// Reached only if $(msbuildprojectdirectory)\foo.cpp was correctly
// recognized as the (case-insensitive) built-in MSBuildProjectDirectory
// property and expanded -- see vcxproj_item_macro_path_case_test.py.
int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
