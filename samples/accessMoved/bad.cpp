void foo(std::string);

int main()
{
    std::string s = "test";
    foo(std::move(s));

    std::cout << s << std::endl;
}
