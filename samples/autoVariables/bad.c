static void foo(int **a)
{
    int b = 1;
    *a = &b;
}

int main()
{
    int *c;
    foo(&c);
    return 0;
}
