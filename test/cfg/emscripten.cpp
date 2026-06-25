
// Test library configuration for emscripten.cfg
//
// Usage:
// $ cppcheck --check-library --library=emscripten --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/emscripten.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <stdio.h>
#include <emscripten.h>

void em_asm_test()
{
    // inline some JavaScript
    EM_ASM(alert('hello'); );
    MAIN_THREAD_EM_ASM(alert('hello main thread'); );

    // pass parameters to JavaScript
    EM_ASM(
    {
        console.log('I received: ' + [$0, $1]);
    },
        100, 35.5);

    // pass a string to JavaScript
    EM_ASM({console.log('hello ' + UTF8ToString($0))}, "world!");
}

void em_asm_int_test()
{
    // cppcheck-suppress unreadVariable
    const int x = EM_ASM_INT({
        return $0 + 42;
    }, 100);

    // cppcheck-suppress unreadVariable
    const int y = MAIN_THREAD_EM_ASM_INT({return 2;});
}

void em_asm_double_test()
{
    // cppcheck-suppress unreadVariable
    const double x = EM_ASM_DOUBLE({
        return $0 + 1.0;
    }, 2.0);

    // cppcheck-suppress unreadVariable
    const double y = MAIN_THREAD_EM_ASM_DOUBLE({return 1.0;});
}

void em_asm_ptr_test()
{
    void* ptr = EM_ASM_PTR({
        return stringToNewUTF8("Hello");
    });
    printf("%s", static_cast<const char*>(ptr));
    free(ptr);
}

void em_asm_ptr_memleak_test()
{
    const char *str = static_cast<char*>(EM_ASM_PTR({
        return stringToNewUTF8("Hello");
    }));
    // cppcheck-suppress nullPointerOutOfMemory
    printf("%s", str);

    // cppcheck-suppress memleak
}

void main_thread_em_asm_ptr_test()
{
    // cppcheck-suppress leakReturnValNotUsed
    MAIN_THREAD_EM_ASM_PTR(
        return stringToNewUTF8("Hello");
        );
}

EM_JS(void, two_alerts, (), {
    alert('hai');
    alert('bai');
});
EM_JS(void, take_args, (int x, float y), {
    console.log('I received: ' + [x, y]);
});

void em_js_test()
{
    two_alerts();
    take_args(100, 35.5);
}
