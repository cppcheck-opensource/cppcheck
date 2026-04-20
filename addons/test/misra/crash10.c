//#12267

extern uint32_t end;

//#define KEEP // if uncomment this then won't crash

KEEP static const int32_t ptr_to_end = &end;

void foo(void)
{
    (void)ptr_to_end;
}
