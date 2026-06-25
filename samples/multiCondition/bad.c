static void f(bool b)
{
    if (b) {}
    else if (!b) {}
}

int main()
{
    f(true);
    return 0;
}
