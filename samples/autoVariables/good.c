static void foo(int **a)
{
    int b = 1;
    **a = b;
}

int main()
{
    int b;
    int *c = &b;
    foo(&c);
    return 0;
}
