// Reached only if $(My-Root)\foo.cpp was correctly recognized as a
// config-invariant property reference and expanded -- which requires the
// hyphen in "My-Root" to be accepted as part of the property name by
// hasOnlyInvariantItemPathVariables()'s inline scanner. See
// vcxproj_item_macro_path_hyphen_test.py.
int main()
{
    int x = 3 / 0; // ERROR
    return x;
}
