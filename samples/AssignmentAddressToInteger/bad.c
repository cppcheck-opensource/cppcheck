static int foo(int *p)
{
    int a = p;
    return a + 4;
}

int main()
{
    int i[10];
    foo(i);
    return 0;
}
