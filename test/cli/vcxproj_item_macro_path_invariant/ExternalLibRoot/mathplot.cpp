// Reached only if $(ExternalLibRoot)\mathplot.cpp was correctly expanded and
// checked -- see vcxproj_item_macro_path_invariant_test.py.
int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
