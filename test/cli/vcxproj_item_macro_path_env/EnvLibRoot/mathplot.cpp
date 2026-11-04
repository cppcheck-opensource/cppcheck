// Reached only if $(CppcheckTestEnvLibRoot)\mathplot.cpp was correctly
// resolved via the CppcheckTestEnvLibRoot environment variable -- see
// vcxproj_item_macro_path_env_test.py.
int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
