
struct S {
    uint32_t some[100];
};

void foo( void )
{
    if (((S *)0x8000)->some[0] != 0U) { }
}

