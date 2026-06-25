
// Test library configuration for sqlite3.cfg
//
// Usage:
// $ cppcheck --check-library --library=sqlite3 --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/sqlite3.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <sqlite3.h>
#include <stdio.h>

void validCode()
{
    sqlite3 * db;

    int rc = sqlite3_open("/db", &db);
    if (rc != SQLITE_OK) {
        printf("Error opening sqlite3 db: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    } else {
        sqlite3_close(db);
    }

    {
        char * buf = sqlite3_malloc(10);
        printf("size: %ull\n", sqlite3_msize(buf));
        sqlite3_free(buf);
    }
}

void memleak_sqlite3_malloc()
{
    char * buf = sqlite3_malloc(10);
    if (buf) {
        buf[0] = 0;
    }
    // cppcheck-suppress memleak
}

void resourceLeak_sqlite3_open()
{
    sqlite3 * db;

    sqlite3_open("/db", &db);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_sqlite3_open_v2(const char* Filename, int Flags, int Timeout, const char* Vfs) { // #12951, don't crash
    sqlite3* handle;
    const int ret = sqlite3_open_v2(Filename, &handle, Flags, Vfs);
    if (SQLITE_OK != ret) {}
    if (Timeout > 0) {}
    // cppcheck-suppress resourceLeak
}

void nullPointer_sqlite3_open_v2(const char* filename, int flags) { // #13078
    sqlite3* handle;
    sqlite3_open_v2(filename, &handle, flags, NULL);
    sqlite3_close_v2(handle);
}

void ignoredReturnValue(const char * buf)
{
    // cppcheck-suppress leakReturnValNotUsed
    sqlite3_malloc(10);
    // cppcheck-suppress leakReturnValNotUsed
    sqlite3_malloc64(5);
    // cppcheck-suppress ignoredReturnValue
    sqlite3_msize(buf);
}
