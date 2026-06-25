class C
{
public:
    explicit C(std::string s)
    : _s(s)
    {
    }
    void foo();
private:
    std::string _s;
};