bool foo(std::string s)
{
    return s.empty();
}

int main()
{
    std::string s;
    foo(s);
}
