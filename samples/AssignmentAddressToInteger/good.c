static int* foo(int *p)
{
    return p + 4;
}

int main()
{
    int i[10];
    foo(i);
    return 0;
}
