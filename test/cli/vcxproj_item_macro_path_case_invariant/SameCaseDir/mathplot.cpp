// Reached only if $(MYROOT)\mathplot.cpp was correctly recognized as the
// same config-invariant property as MyRoot/myroot and expanded -- see
// vcxproj_item_macro_path_case_test.py.
int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
