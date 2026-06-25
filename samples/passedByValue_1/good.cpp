class C
{
public:
    explicit C(std::string s)
    : _s(std::move(s))
    {
    }
    void foo();
private:
    std::string _s;
};