void foo(const std::string&, const std::string&);

int main()
{
    std::string s1 = "test1", s2 = "test2";
    foo(s1, s1);
}
