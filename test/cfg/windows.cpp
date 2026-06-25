
// Test library configuration for windows.cfg
//
// Usage:
// $ cppcheck --check-library --library=windows --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/windows.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file [valueFlowBailout,purgedConfiguration]

#include <Windows.h>
#include <WinCon.h>
#include <SetupAPI.h>
#include <cstdio>
#include <direct.h>
#include <evntrace.h>
#include <cstdlib>
#include <ctime>
#include <memory.h>
#include <mbstring.h>
#include <tchar.h>
#include <wchar.h>
#include <atlstr.h>
#include <string>

bool UpdateTraceACalled(TRACEHANDLE traceHandle, LPCSTR loggerName, EVENT_TRACE_PROPERTIES* pProperties)
{
    // cppcheck-suppress UpdateTraceACalled
    return UpdateTraceA(traceHandle, loggerName, pProperties) != ERROR_SUCCESS;
}
bool UpdateTraceWCalled(TRACEHANDLE traceHandle, LPCWSTR loggerName, EVENT_TRACE_PROPERTIES* pProperties)
{
    // cppcheck-suppress UpdateTraceWCalled
    return UpdateTraceW(traceHandle, loggerName, pProperties) != ERROR_SUCCESS;
}

void invalidHandle_CreateFile(LPCWSTR lpFileName)
{
    HANDLE file = CreateFile(lpFileName, GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    // INVALID_HANDLE_VALUE is not the same as 0
    if (file != INVALID_HANDLE_VALUE && file) {}

    // cppcheck-suppress resourceLeak
}

void invalid_socket()
{
    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

    // INVALID_SOCKET is not the same as 0
    if (sock != INVALID_SOCKET && sock) {}

    // cppcheck-suppress resourceLeak
}

void invalid_socket_noleak(int a, int t, int p) { // #14155
    SOCKET s = socket(a, t, p);
    if (s == INVALID_SOCKET)
        return;
    closesocket(s);
}

void resourceLeak_OpenThread(const DWORD dwDesiredAccess, const BOOL bInheritHandle, const DWORD dwThreadId)
{
    HANDLE proc = OpenThread(dwDesiredAccess, bInheritHandle, dwThreadId);
    if (proc != INVALID_HANDLE_VALUE) {}
    // cppcheck-suppress resourceLeak
}

void resourceLeak_OpenProcess(const DWORD dwDesiredAccess, const BOOL bInheritHandle, const DWORD dwProcessId)
{
    HANDLE proc = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    if (proc != INVALID_HANDLE_VALUE) {}
    // cppcheck-suppress resourceLeak
}

/// https://learn.microsoft.com/en-us/windows/console/flushconsoleinputbuffer
BOOL unreachableCode_FlushConsoleInputBuffer(int &val)
{
    const BOOL retVal = FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    // still reachable after call FlushConsoleInputBuffer()
    val = 42;
    return retVal;
}

/// https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew
std::string constVariable_GetModuleFileName(void) {
    char path[42];
    if (GetModuleFileNameA(NULL, path, sizeof(path))==0)
        return std::string();
    return std::string{path};
}

const TCHAR* constVariable_MAKEINTRESOURCE() { // #14564
    return MAKEINTRESOURCE(5 - 1);
}

int stringCompare_mbscmp(const unsigned char *string1, const unsigned char *string2)
{
    // cppcheck-suppress stringCompare
    (void) _mbscmp(string1, string1);
    const unsigned char x[] = "x";
    // cppcheck-suppress stringCompare
    (void) _mbscmp(x, x);
    return _mbscmp(string1, string2);
}

int stringCompare_mbscmp_l(const unsigned char *string1, const unsigned char *string2, _locale_t locale)
{
    // cppcheck-suppress stringCompare
    (void) _mbscmp_l(string1, string1, locale);
    const unsigned char x[] = "x";
    // cppcheck-suppress stringCompare
    (void) _mbscmp_l(x, x, locale);
    return _mbscmp_l(string1, string2, locale);
}

int ignoredReturnValue__wtoi_l(const wchar_t *str, _locale_t locale)
{
    // cppcheck-suppress ignoredReturnValue
    _wtoi_l(str,locale);
    return _wtoi_l(str,locale);
}

int ignoredReturnValue__atoi_l(const char *str, _locale_t locale)
{
    // cppcheck-suppress ignoredReturnValue
    _atoi_l(str,locale);
    return _atoi_l(str,locale);
}

void invalidFunctionArg__fseeki64(FILE* stream, __int64 offset, int origin)
{
    // cppcheck-suppress invalidFunctionArg
    (void)_fseeki64(stream, offset, -1);
    // cppcheck-suppress invalidFunctionArg
    (void)_fseeki64(stream, offset, 3);
    // cppcheck-suppress invalidFunctionArg
    (void)_fseeki64(stream, offset, 42+SEEK_SET);
    // cppcheck-suppress invalidFunctionArg
    (void)_fseeki64(stream, offset, SEEK_SET+42);
    // No warning is expected for
    (void)_fseeki64(stream, offset, origin);
    (void)_fseeki64(stream, offset, SEEK_SET);
    (void)_fseeki64(stream, offset, SEEK_CUR);
    (void)_fseeki64(stream, offset, SEEK_END);
}

void invalidFunctionArgBool__fseeki64(FILE* stream, __int64 offset)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)_fseeki64(stream, offset, true);
    // cppcheck-suppress invalidFunctionArgBool
    (void)_fseeki64(stream, offset, false);
}

unsigned char * overlappingWriteFunction__mbscat(unsigned char *src, unsigned char *dest)
{
    // No warning shall be shown:
    (void)_mbscat(dest, src);
    // cppcheck-suppress overlappingWriteFunction
    return _mbscat(src, src);
}

void* overlappingWriteFunction__memccpy(const unsigned char *src, unsigned char *dest, int c, size_t count)
{
    // No warning shall be shown:
    (void)_memccpy(dest, src, c, count);
    (void)_memccpy(dest, src, 42, count);
    // cppcheck-suppress overlappingWriteFunction
    (void) _memccpy(dest, dest, c, 4);
    // cppcheck-suppress overlappingWriteFunction
    return _memccpy(dest, dest+3, c, 4);
}

unsigned char * overlappingWriteFunction__mbscpy(unsigned char *src, unsigned char *dest)
{
    // No warning shall be shown:
    (void)_mbscpy(dest, src);
    // cppcheck-suppress overlappingWriteFunction
    return _mbscpy(src, src);
}

void overlappingWriteFunction__swab(char *src, char *dest, int n)
{
    // No warning shall be shown:
    _swab(dest, src, n);
    // cppcheck-suppress overlappingWriteFunction
    _swab(src, src+3, 4);
}

SYSTEM_INFO uninitvar_GetSystemInfo()
{
    // No warning is expected
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    return SystemInfo;
}

void uninitvar__putenv(const char * envstr)
{
    // No warning is expected
    (void)_putenv(envstr);

    const char * p;
    // cppcheck-suppress uninitvar
    (void)_putenv(p);
}

void nullPointer__putenv(const char * envstr)
{
    // No warning is expected
    (void)_putenv(envstr);

    const char * p=NULL;
    // cppcheck-suppress nullPointer
    (void)_putenv(p);
}

void invalidFunctionArg__getcwd(char * buffer)
{
    // Passing NULL as the buffer forces getcwd to allocate
    // memory for the path, which allows the code to support file paths
    // longer than _MAX_PATH, which are supported by NTFS.
    if ((buffer = _getcwd(NULL, 0)) == NULL) {
        return;
    }
    free(buffer);
}
// DWORD GetPrivateProfileString(
//  [in]  LPCTSTR lpAppName,
//  [in]  LPCTSTR lpKeyName,
//  [in]  LPCTSTR lpDefault,
//  [out] LPTSTR  lpReturnedString,
//  [in]  DWORD   nSize,
//  [in]  LPCTSTR lpFileName)
void nullPointer_GetPrivateProfileString(LPCTSTR lpAppName,
                                         LPCTSTR lpKeyName,
                                         LPCTSTR lpDefault,
                                         LPTSTR lpReturnedString,
                                         DWORD nSize,
                                         LPCTSTR lpFileName)
{
    // No warning is expected
    (void)GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);

    // No warning is expected for 1st arg as nullptr
    (void)GetPrivateProfileString(nullptr, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
    // No warning is expected for 2nd arg as nullptr
    (void)GetPrivateProfileString(lpAppName, nullptr, lpDefault, lpReturnedString, nSize, lpFileName);
}

void nullPointer__get_timezone(long *sec)
{
    // No warning is expected
    (void)_get_timezone(sec);

    long *pSec = NULL;
    // cppcheck-suppress nullPointer
    (void)_get_timezone(pSec);
}

void nullPointer__get_daylight(int *h)
{
    // No warning is expected
    (void)_get_daylight(h);

    int *pHours = NULL;
    // cppcheck-suppress nullPointer
    (void)_get_daylight(pHours);
}

void validCode()
{
    DWORD dwordInit = 0;
    WORD wordInit = 0;
    BYTE byteInit = 0;

    // Valid Semaphore usage, no leaks, valid arguments
    HANDLE hSemaphore1;
    hSemaphore1 = CreateSemaphore(NULL, 0, 1, NULL);
    CloseHandle(hSemaphore1);
    HANDLE hSemaphore2;
    // cppcheck-suppress valueFlowBailoutIncompleteVar
    hSemaphore2 = CreateSemaphoreEx(NULL, 0, 1, NULL, 0, SEMAPHORE_ALL_ACCESS);
    CloseHandle(hSemaphore2);
    HANDLE hSemaphore3;
    hSemaphore3 = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, L"sem");
    CloseHandle(hSemaphore3);

    // Valid lstrcat usage, but with warning because it is deprecated
    char buf[30] = "hello world";
    // cppcheck-suppress lstrcatACalled
    lstrcatA(buf, "test");

    // cppcheck-suppress strlwrCalled
    strlwr(buf);
    // cppcheck-suppress struprCalled
    strupr(buf);

    // Valid Mutex usage, no leaks, valid arguments
    HANDLE hMutex1;
    hMutex1 = CreateMutex(NULL, TRUE, NULL);
    if (hMutex1) {
        ReleaseMutex(hMutex1);
    }
    CloseHandle(hMutex1);
    HANDLE hMutex2;
    hMutex2 = CreateMutexEx(NULL, NULL, 0, MUTEX_ALL_ACCESS);
    CloseHandle(hMutex2);
    HANDLE hMutex3;
    hMutex3 = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T("sem"));
    CloseHandle(hMutex3);

    // Valid Module usage, no leaks, valid arguments
    HMODULE hModule = GetModuleHandle(L"My.dll");
    FreeLibrary(hModule);
    hModule = GetModuleHandle(TEXT("somedll"));
    FreeLibrary(hModule);
    hModule = GetModuleHandle(NULL);
    FreeLibrary(hModule);

    // Valid Event usage, no leaks, valid arguments
    HANDLE event;
    event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL != event) {
        SetEvent(event);
        CloseHandle(event);
    }
    event = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"testevent");
    if (NULL != event) {
        PulseEvent(event);
        SetEvent(event);
        CloseHandle(event);
    }
    event = CreateEventEx(NULL, L"testevent3", CREATE_EVENT_INITIAL_SET, EVENT_MODIFY_STATE);
    if (NULL != event) {
        ResetEvent(event);
        CloseHandle(event);
    }

    // cppcheck-suppress unusedAllocatedMemory
    void *pMem1 = _malloca(1);
    _freea(pMem1);
    // Memory from _alloca must not be freed
    // cppcheck-suppress _allocaCalled
    void *pMem2 = _alloca(10);
    memset(pMem2, 0, 10);

    SYSTEMTIME st;
    GetSystemTime(&st);

    DWORD lastError = GetLastError();
    SetLastError(lastError);

    PSID pEveryoneSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID);
    FreeSid(pEveryoneSID);

    LPVOID pMem = HeapAlloc(GetProcessHeap(), 0, 10);
    pMem = HeapReAlloc(GetProcessHeap(), 0, pMem, 0);
    HeapFree(GetProcessHeap(), 0, pMem);

    char bufC[50];
    sprintf_s(bufC, "Hello");
    printf("%s", bufC);
    sprintf_s(bufC, "%s", "test");
    printf("%s", bufC);
    sprintf_s(bufC, _countof(bufC), "%s", "test");
    printf("%s", bufC);
    wchar_t bufWC[50];
    swprintf_s(bufWC, L"Hello");
    wprintf(L"%s\n", bufWC);
    swprintf_s(bufWC, L"%s %d", L"swprintf_s", 3);
    wprintf(L"%s\n", bufWC);
    swprintf_s(bufWC, _countof(bufWC), L"%s %d", L"swprintf_s", 6);
    wprintf(L"%s\n", bufWC);
    TCHAR bufTC[50];
    _stprintf(bufTC, TEXT("Hello"));
    _tprintf(TEXT("%s"), bufTC);
    _stprintf(bufTC, TEXT("%d"), 1);
    _tprintf(TEXT("%s"), bufTC);
    _stprintf(bufTC, TEXT("%d"), 2);
    _tprintf(TEXT("%s"), bufTC);

    GetUserName(NULL, &dwordInit);
    dwordInit = 10;
    GetUserName(bufTC, &dwordInit);

    WSADATA wsaData = {0};
    WSAStartup(2, &wsaData);
    SOCKET sock = socket(1, 2, 3);
    u_long ulongInit = 0;
    ioctlsocket(sock, FIONBIO, &ulongInit);
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();

    wordInit = MAKEWORD(1, 2);
    // TODO cppcheck-suppress redundantAssignment
    dwordInit = MAKELONG(1, 2);
    // cppcheck-suppress redundantAssignment
    wordInit = LOWORD(dwordInit);
    byteInit = LOBYTE(wordInit);
    wordInit = HIWORD(dwordInit);
    // cppcheck-suppress redundantAssignment
    byteInit = HIBYTE(wordInit);
    // cppcheck-suppress knownConditionTrueFalse
    if (byteInit) {}

    bool boolVar;
    uint8_t byteBuf[5] = {0};
    uint8_t byteBuf2[10] = {0};
    boolVar = RtlEqualMemory(byteBuf, byteBuf2, sizeof(byteBuf));
    if (boolVar) {}
    boolVar = RtlCompareMemory(byteBuf, byteBuf2, sizeof(byteBuf));
    if (boolVar) {}
    RtlMoveMemory(byteBuf, byteBuf2, sizeof(byteBuf));
    RtlCopyMemory(byteBuf, byteBuf2, sizeof(byteBuf));
    RtlZeroMemory(byteBuf, sizeof(byteBuf));
    ZeroMemory(byteBuf, sizeof(byteBuf));
    RtlSecureZeroMemory(byteBuf, sizeof(byteBuf));
    SecureZeroMemory(byteBuf, sizeof(byteBuf));
    RtlFillMemory(byteBuf, sizeof(byteBuf), 0xff);

    // cppcheck-suppress [LocalAllocCalled, unusedAllocatedMemory]
    HLOCAL pLocalAlloc = LocalAlloc(1, 2);
    LocalFree(pLocalAlloc);

    // cppcheck-suppress lstrlenCalled
    (void)lstrlen(bufTC);
    // cppcheck-suppress lstrlenCalled
    (void)lstrlen(NULL);

    // Intrinsics
    __noop();
    __noop(1, "test", NULL);
    __nop();

    // cppcheck-suppress unusedAllocatedMemory
    void * pAlloc1 = _aligned_malloc(100, 2);
    _aligned_free(pAlloc1);

    ::PostMessage(nullptr, WM_QUIT, 0, 0);

    printf("%zu", __alignof(int));
    printf("%zu", _alignof(double));

    // Valid Library usage, no leaks, valid arguments
    HINSTANCE hInstLib = LoadLibrary(L"My.dll");
    FreeLibrary(hInstLib);
    hInstLib = LoadLibraryA("My.dll");
    FreeLibrary(hInstLib);
    hInstLib = LoadLibraryEx(L"My.dll", NULL, 0);
    FreeLibrary(hInstLib);
    hInstLib = LoadLibraryExW(L"My.dll", NULL, 0);
    FreeLibrary(hInstLib);
    hInstLib = ::LoadLibrary(L"My.dll");
    FreeLibraryAndExitThread(hInstLib, 0); // Does not return! Must be at the end!
}

void bufferAccessOutOfBounds()
{
    wchar_t buf[10];
    // Verifying _countof macro configuration
    // Valid loop over array
    for (size_t i = 0; i < _countof(buf); ++i) {
        buf[i] = L'\0';
    }
    // Wrong loop over array accessing one element past the end
    for (size_t i = 0; i <= _countof(buf); ++i) {
        // cppcheck-suppress arrayIndexOutOfBounds
        buf[i] = L'\0';
    }

    uint8_t byteBuf[5] = {0};
    uint8_t byteBuf2[10] = {0};
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlEqualMemory(byteBuf, byteBuf2, 20);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlCompareMemory(byteBuf, byteBuf2, 20);
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlMoveMemory(byteBuf, byteBuf2, 20);
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress bufferAccessOutOfBounds
    MoveMemory(byteBuf, byteBuf2, 20);
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlCopyMemory(byteBuf, byteBuf2, 20);
    // TODO cppcheck-suppress redundantCopy
    // cppcheck-suppress bufferAccessOutOfBounds
    CopyMemory(byteBuf, byteBuf2, 20);
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlZeroMemory(byteBuf, sizeof(byteBuf)+1);
    // cppcheck-suppress bufferAccessOutOfBounds
    ZeroMemory(byteBuf, sizeof(byteBuf)+1);
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlSecureZeroMemory(byteBuf, sizeof(byteBuf)+1);
    // cppcheck-suppress bufferAccessOutOfBounds
    SecureZeroMemory(byteBuf, sizeof(byteBuf)+1);
    // cppcheck-suppress bufferAccessOutOfBounds
    RtlFillMemory(byteBuf, sizeof(byteBuf)+1, 0x01);
    // cppcheck-suppress bufferAccessOutOfBounds
    FillMemory(byteBuf, sizeof(byteBuf)+1, 0x01);

    char * pAlloc1 = static_cast<char*>(_malloca(32));
    // cppcheck-suppress nullPointerOutOfMemory
    memset(pAlloc1, 0, 32);
    // cppcheck-suppress bufferAccessOutOfBounds
    // cppcheck-suppress nullPointerOutOfMemory
    memset(pAlloc1, 0, 33);
    _freea(pAlloc1);
}

void mismatchAllocDealloc()
{
    char * pChar = static_cast<char*>(_aligned_malloc(100, 2));
    // cppcheck-suppress mismatchAllocDealloc
    free(pChar);

    // cppcheck-suppress unusedAllocatedMemory
    pChar = static_cast<char*>(_malloca(32));
    // cppcheck-suppress mismatchAllocDealloc
    _aligned_free(pChar);
}

void nullPointer()
{
    HANDLE hSemaphore;
    // cppcheck-suppress [nullPointer,valueFlowBailoutIncompleteVar]
    hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, NULL);
    CloseHandle(hSemaphore);

    // cppcheck-suppress lstrcatCalled
    // cppcheck-suppress nullPointer
    lstrcat(NULL, _T("test"));
    TCHAR buf[10] = _T("\0");
    // cppcheck-suppress lstrcatCalled
    // cppcheck-suppress nullPointer
    lstrcat(buf, NULL);

    HANDLE hMutex;
    // cppcheck-suppress nullPointer
    hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, NULL);
    CloseHandle(hMutex);

    //Incorrect: 1. parameter, must not be null
    // cppcheck-suppress nullPointer
    FARPROC pAddr = GetProcAddress(NULL, "name");
    (void)pAddr;
    HMODULE * phModule = NULL;
    // cppcheck-suppress nullPointer
    GetModuleHandleEx(0, NULL, phModule);

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    OpenEvent(EVENT_ALL_ACCESS, FALSE, NULL);
    HANDLE hEvent = NULL;
    // cppcheck-suppress nullPointer
    PulseEvent(hEvent);
    // cppcheck-suppress nullPointer
    ResetEvent(hEvent);
    // cppcheck-suppress nullPointer
    SetEvent(hEvent);

    char *str = NULL;
    // cppcheck-suppress strlwrCalled
    // cppcheck-suppress nullPointer
    strlwr(str);
    // cppcheck-suppress struprCalled
    // cppcheck-suppress nullPointer
    strupr(str);

    // cppcheck-suppress nullPointer
    GetSystemTime(NULL);
    // cppcheck-suppress nullPointer
    GetLocalTime(NULL);

    // TODO: error message: arg1 must not be nullptr if variable pointed to by arg2 is not 0
    DWORD dwordInit = 10;
    GetUserName(NULL, &dwordInit);
    TCHAR bufTC[10];
    // cppcheck-suppress nullPointer
    GetUserName(bufTC, NULL);

    SOCKET socketInit = {0};
    sockaddr sockaddrUninit;
    int intInit = 0;
    int *pIntNull = NULL;
    char charArray[] = "test";
    // cppcheck-suppress nullPointer
    WSAStartup(1, NULL);
    // cppcheck-suppress nullPointer
    bind(socketInit, NULL, 5);
    // cppcheck-suppress nullPointer
    getpeername(socketInit, NULL, &intInit);
    // cppcheck-suppress nullPointer
    getpeername(socketInit, &sockaddrUninit, pIntNull);
    // cppcheck-suppress nullPointer
    getsockopt(socketInit, 1, 2, NULL, &intInit);
    // cppcheck-suppress nullPointer
    getsockopt(socketInit, 1, 2, charArray, pIntNull);
}

void memleak_malloca()
{
    // cppcheck-suppress [unusedAllocatedMemory, unreadVariable, constVariablePointer]
    void *pMem = _malloca(10);
    // cppcheck-suppress memleak
}

void memleak_AllocateAndInitializeSid()
{
    PSID pEveryoneSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID);
    // cppcheck-suppress memleak
}

void memleak_HeapAlloc()
{
    LPVOID pMem;
    pMem = HeapAlloc(GetProcessHeap(), 0, 10);
    HeapValidate(GetProcessHeap(), 0, pMem);
    // cppcheck-suppress unreadVariable
    SIZE_T memSize = HeapSize(GetProcessHeap(), 0, pMem);
    // cppcheck-suppress memleak
}

void memleak_LocalAlloc()
{
    LPTSTR pszBuf;
    // cppcheck-suppress [LocalAllocCalled, cstyleCast, valueFlowBailoutIncompleteVar]
    pszBuf = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    (void)LocalSize(pszBuf);
    (void)LocalFlags(pszBuf);
    LocalLock(pszBuf);
    LocalUnlock(pszBuf);
    // cppcheck-suppress memleak
}

void memleak_dupenv_s() // #10646
{
    char* pValue;
    size_t len;
    errno_t err = _dupenv_s(&pValue, &len, "pathext");
    if (err) return;
    printf("pathext = %s\n", pValue);
    free(pValue);
    err = _dupenv_s(&pValue, &len, "nonexistentvariable");
    if (err) return;
    printf("nonexistentvariable = %s\n", pValue);
    // cppcheck-suppress memleak
}

void resourceLeak_CreateSemaphoreA()
{
    HANDLE hSemaphore;
    // cppcheck-suppress unreadVariable
    hSemaphore = CreateSemaphoreA(NULL, 0, 1, "sem1");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateSemaphoreEx()
{
    HANDLE hSemaphore;
    // cppcheck-suppress [unreadVariable,valueFlowBailoutIncompleteVar]
    hSemaphore = CreateSemaphoreEx(NULL, 0, 1, NULL, 0, SEMAPHORE_ALL_ACCESS);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_OpenSemaphore()
{
    HANDLE hSemaphore;
    // cppcheck-suppress [unreadVariable,valueFlowBailoutIncompleteVar]
    hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, _T("sem"));
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateMutexA()
{
    HANDLE hMutex;
    // cppcheck-suppress unreadVariable
    hMutex = CreateMutexA(NULL, TRUE, "sem1");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateMutexEx()
{
    HANDLE hMutex;
    // cppcheck-suppress [unreadVariable,valueFlowBailoutIncompleteVar]
    hMutex = CreateMutexEx(NULL, _T("sem"), 0, MUTEX_ALL_ACCESS);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_OpenMutex()
{
    HANDLE hMutex;
    // cppcheck-suppress unreadVariable
    hMutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, _T("sem"));
    // cppcheck-suppress resourceLeak
}

void resourceLeak_LoadLibrary()
{
    HINSTANCE hInstLib;
    hInstLib = ::LoadLibrary(L"My.dll");
    typedef BOOL (WINAPI *fpFunc)();
    // cppcheck-suppress [unreadVariable, cstyleCast]
    fpFunc pFunc = (fpFunc)GetProcAddress(hInstLib, "name");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateEvent()
{
    HANDLE hEvent;
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    SetEvent(hEvent);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_CreateEventExA()
{
    HANDLE hEvent;
    // cppcheck-suppress unreadVariable
    hEvent = CreateEventExA(NULL, "test", CREATE_EVENT_INITIAL_SET, EVENT_MODIFY_STATE);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_OpenEventW()
{
    HANDLE hEvent;
    // cppcheck-suppress [unreadVariable,valueFlowBailoutIncompleteVar]
    hEvent = OpenEventW(EVENT_ALL_ACCESS, TRUE, L"testevent");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_socket()
{
    SOCKET sock;
    // cppcheck-suppress unreadVariable
    sock = socket(1, 2, 3);
    // cppcheck-suppress resourceLeak
}

void ignoredReturnValue(FILE* fp)
{
    // cppcheck-suppress leakReturnValNotUsed
    CreateSemaphoreW(NULL, 0, 1, NULL);
    // cppcheck-suppress [leakReturnValNotUsed,valueFlowBailoutIncompleteVar]
    CreateSemaphoreExA(NULL, 0, 1, NULL, 0, SEMAPHORE_ALL_ACCESS);
    // cppcheck-suppress leakReturnValNotUsed
    OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, TRUE, "sem");

    // cppcheck-suppress leakReturnValNotUsed
    CreateMutexW(NULL, FALSE, NULL);
    // cppcheck-suppress leakReturnValNotUsed
    CreateMutexExA(NULL, NULL, 1, MUTEX_ALL_ACCESS);
    // cppcheck-suppress leakReturnValNotUsed
    OpenMutexA(MUTEX_ALL_ACCESS, TRUE, "sem");

    // cppcheck-suppress leakReturnValNotUsed
    LoadLibrary(L"My.dll");
    // cppcheck-suppress leakReturnValNotUsed
    LoadLibraryEx(L"My.dll", NULL, 0);

    HINSTANCE hInstLib = LoadLibrary(L"My.dll");
    // cppcheck-suppress ignoredReturnValue
    GetProcAddress(hInstLib, "name");
    FreeLibrary(hInstLib);

    // cppcheck-suppress leakReturnValNotUsed
    CreateEvent(NULL, FALSE, FALSE, NULL);
    // cppcheck-suppress leakReturnValNotUsed
    OpenEvent(EVENT_ALL_ACCESS, FALSE, L"testevent");
    // cppcheck-suppress leakReturnValNotUsed
    CreateEventEx(NULL, L"test", CREATE_EVENT_INITIAL_SET, EVENT_MODIFY_STATE);

    // cppcheck-suppress leakReturnValNotUsed
    _malloca(10);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress _allocaCalled
    _alloca(5);

    // cppcheck-suppress ignoredReturnValue
    GetLastError();

    // cppcheck-suppress ignoredReturnValue
    GetProcessHeap();
    // cppcheck-suppress leakReturnValNotUsed
    HeapAlloc(GetProcessHeap(), 0, 10);
    // cppcheck-suppress [leakReturnValNotUsed, nullPointer]
    HeapReAlloc(GetProcessHeap(), 0, nullptr, 0);

    // cppcheck-suppress leakReturnValNotUsed
    socket(1, 2, 3);

    // cppcheck-suppress ignoredReturnValue
    _fileno(fp);

    // cppcheck-suppress lstrlenCalled
    // cppcheck-suppress ignoredReturnValue
    lstrlen(TEXT("test"));
}

void invalidFunctionArg()
{
    HANDLE hSemaphore;
    // cppcheck-suppress invalidFunctionArg
    hSemaphore = CreateSemaphore(NULL, 0, 0, NULL);
    CloseHandle(hSemaphore);
    // cppcheck-suppress invalidFunctionArgBool
    hSemaphore = CreateSemaphore(NULL, 0, 1, false);
    CloseHandle(hSemaphore);
    // cppcheck-suppress [invalidFunctionArg,valueFlowBailoutIncompleteVar]
    hSemaphore = CreateSemaphoreEx(NULL, 0, 0, NULL, 0, SEMAPHORE_ALL_ACCESS);
    CloseHandle(hSemaphore);
    // cppcheck-suppress invalidFunctionArg
    hSemaphore = CreateSemaphoreEx(NULL, 0, 1, NULL, 1, SEMAPHORE_ALL_ACCESS);
    CloseHandle(hSemaphore);

    HANDLE hMutex;
    // cppcheck-suppress invalidFunctionArgBool
    hMutex = CreateMutex(NULL, TRUE, false);
    CloseHandle(hMutex);
    // cppcheck-suppress invalidFunctionArgBool
    hMutex = CreateMutex(NULL, FALSE, false);
    CloseHandle(hMutex);
    // cppcheck-suppress invalidFunctionArg
    hMutex = CreateMutexEx(NULL, NULL, 3, MUTEX_ALL_ACCESS);
    CloseHandle(hMutex);

    //Incorrect: 2. parameter to LoadLibraryEx() must be NULL
    // cppcheck-suppress [invalidFunctionArg, intToPointerCast]
    HINSTANCE hInstLib = LoadLibraryEx(L"My.dll", HANDLE(1), 0);
    FreeLibrary(hInstLib);

    // cppcheck-suppress invalidFunctionArg
    void *pMem = _malloca(-1);
    _freea(pMem);
    // FIXME cppcheck-suppress unreadVariable
    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress _allocaCalled
    pMem = _alloca(-5);
}

void uninitvar()
{
    // cppcheck-suppress unassignedVariable
    HANDLE hSemaphore;
    // cppcheck-suppress uninitvar
    CloseHandle(hSemaphore);

    TCHAR buf[10];
    // cppcheck-suppress lstrcatCalled
    // cppcheck-suppress uninitvar
    lstrcat(buf, _T("test"));
    buf[0] = _T('\0');
    // TODO cppcheck-suppress constVariable
    TCHAR buf2[2];
    // cppcheck-suppress lstrcatCalled
    // cppcheck-suppress uninitvar
    lstrcat(buf, buf2);

    // cppcheck-suppress unassignedVariable
    HANDLE hMutex1, hMutex2;
    // cppcheck-suppress uninitvar
    ReleaseMutex(hMutex1);
    // cppcheck-suppress uninitvar
    CloseHandle(hMutex2);

    // cppcheck-suppress unassignedVariable
    HANDLE hEvent1, hEvent2, hEvent3, hEvent4;
    // cppcheck-suppress uninitvar
    PulseEvent(hEvent1);
    // cppcheck-suppress uninitvar
    ResetEvent(hEvent2);
    // cppcheck-suppress uninitvar
    SetEvent(hEvent3);
    // cppcheck-suppress uninitvar
    CloseHandle(hEvent4);

    char buf_uninit1[10];
    char buf_uninit2[10];
    // cppcheck-suppress strlwrCalled
    // cppcheck-suppress uninitvar
    strlwr(buf_uninit1);
    // cppcheck-suppress struprCalled
    // cppcheck-suppress uninitvar
    strupr(buf_uninit2);

    DWORD dwordUninit;
    // cppcheck-suppress uninitvar
    SetLastError(dwordUninit);

    DWORD dwordUninit2;
    // cppcheck-suppress uninitvar
    GetUserName(NULL, &dwordUninit2);

    FILE *pFileUninit;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress ignoredReturnValue
    _fileno(pFileUninit);
}

void unreferencedParameter(int i) {
    UNREFERENCED_PARAMETER(i);
}

void errorPrintf()
{
    char bufC[50];
    // cppcheck-suppress wrongPrintfScanfArgNum
    sprintf_s(bufC, _countof(bufC), "%s %d", "sprintf_s");
    printf("%s\n", bufC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    sprintf_s(bufC, "%s %d", "sprintf_s");
    printf("%s\n", bufC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    sprintf_s(bufC, _countof(bufC), "test", 0);
    printf("%s\n", bufC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    sprintf_s(bufC, "test", "sprintf_s");
    printf("%s\n", bufC);
    // cppcheck-suppress invalidPrintfArgType_s
    sprintf_s(bufC, _countof(bufC), "%s", 1);
    printf("%s\n", bufC);
    // cppcheck-suppress invalidPrintfArgType_s
    sprintf_s(bufC, "%s", 1);
    printf("%s\n", bufC);

    wchar_t bufWC[50];
    // cppcheck-suppress wrongPrintfScanfArgNum
    swprintf_s(bufWC, _countof(bufWC), L"%s %d", L"swprintf_s");
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    swprintf_s(bufWC, L"%s %d", L"swprintf_s");
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    swprintf_s(bufWC, _countof(bufWC), L"test", 0);
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    swprintf_s(bufWC, L"test", L"swprintf_s");
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress invalidPrintfArgType_s
    swprintf_s(bufWC, _countof(bufWC), L"%s", 1);
    wprintf(L"%s\n", bufWC);
    // cppcheck-suppress invalidPrintfArgType_s
    swprintf_s(bufWC, L"%s", 1);
    wprintf(L"%s\n", bufWC);

    TCHAR bufTC[50];
    // cppcheck-suppress wrongPrintfScanfArgNum
    _stprintf_s(bufTC, _countof(bufTC), TEXT("%s %d"), TEXT("_stprintf_s"));
    _tprintf(L"%s\n", bufTC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    _stprintf_s(bufTC, TEXT("%s %d"), TEXT("_stprintf_s"));
    _tprintf(TEXT("%s\n"), bufTC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    _stprintf_s(bufTC, _countof(bufTC), TEXT("test"), 0);
    _tprintf(TEXT("%s\n"), bufTC);
    // cppcheck-suppress wrongPrintfScanfArgNum
    _stprintf_s(bufTC, TEXT("test"), TEXT("_stprintf_s"));
    _tprintf(TEXT("%s\n"), bufTC);
    // cppcheck-suppress invalidPrintfArgType_s
    _stprintf_s(bufTC, _countof(bufTC), TEXT("%s"), 1);
    _tprintf(TEXT("%s\n"), bufTC);
    // cppcheck-suppress invalidPrintfArgType_s
    _stprintf_s(bufTC, TEXT("%s"), 1);
    _tprintf(TEXT("%s\n"), bufTC);
}

void allocDealloc_GetModuleHandleEx()
{
    // For GetModuleHandleEx it depends on the first argument if FreeLibrary
    // must be called or is not allowed to be called.
    // If the first argument is GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
    // (0x00000002), a call to FreeLibrary is not allowed, otherwise it is
    // necessary.
    // Since this is not possible to configure at the moment cppcheck should
    // accept not calling FreeLibrary and also calling it for the handle.
    // TODO: Enhance cppcheck to conditionally check for alloc/dealloc issues.

    // No warning because of correct FreeLibrary on 'hModule' should be issued.
    HMODULE hModule1;
    if (GetModuleHandleEx(0, NULL, &hModule1)) {
        FreeLibrary(hModule1);
    }

    //This is a false negative, but it is not detected to avoid false positives.
    HMODULE hModule2;
    GetModuleHandleEx(0, NULL, &hModule2);
}

void uninitvar_tolower(_locale_t l)
{
    int c1, c2;
    // cppcheck-suppress uninitvar
    (void)_tolower(c1);
    // cppcheck-suppress uninitvar
    (void)_tolower_l(c2, l);
}

void uninitvar_toupper(_locale_t l)
{
    int c1, c2;
    // cppcheck-suppress uninitvar
    (void)_toupper(c1);
    // cppcheck-suppress uninitvar
    (void)_toupper_l(c2, l);
}

void uninitvar_towlower(_locale_t l)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)_towlower_l(i, l);
}

void uninitvar_towupper(_locale_t l)
{
    wint_t i;
    // cppcheck-suppress uninitvar
    (void)_towupper_l(i, l);
}

void oppositeInnerCondition_SUCCEEDED_FAILED(HRESULT hr)
{
    if (SUCCEEDED(hr)) {
        // TODO ticket #8596 cppcheck-suppress oppositeInnerCondition
        if (FAILED(hr)) {}
    }
}

/*HANDLE WINAPI CreateThread(
   _In_opt_  LPSECURITY_ATTRIBUTES  lpThreadAttributes,
   _In_      SIZE_T                 dwStackSize,
   _In_      LPTHREAD_START_ROUTINE lpStartAddress,
   _In_opt_  LPVOID                 lpParameter,
   _In_      DWORD                  dwCreationFlags,
   _Out_opt_ LPDWORD                lpThreadId
   );*/
HANDLE test_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes,
                         SIZE_T dwStackSize,
                         LPTHREAD_START_ROUTINE lpStartAddress,
                         LPVOID lpParameter,
                         DWORD dwCreationFlags,
                         LPDWORD lpThreadId)
{
    // Create uninitialized variables
    LPSECURITY_ATTRIBUTES uninit_lpThreadAttributes;
    SIZE_T uninit_dwStackSize;
    LPTHREAD_START_ROUTINE uninit_lpStartAddress;
    LPVOID uninit_lpParameter;
    DWORD uninit_dwCreationFlags;

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress uninitvar
    (void) CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, uninit_dwCreationFlags, lpThreadId);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress uninitvar
    (void) CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, uninit_lpParameter, dwCreationFlags, lpThreadId);
    // @todo uninitvar shall be reported
    // cppcheck-suppress leakReturnValNotUsed
    (void) CreateThread(lpThreadAttributes, dwStackSize, uninit_lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress uninitvar
    (void) CreateThread(lpThreadAttributes, uninit_dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
    // @todo uninitvar shall be reported
    // cppcheck-suppress leakReturnValNotUsed
    (void) CreateThread(uninit_lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    (void) CreateThread(lpThreadAttributes, dwStackSize, 0, lpParameter, dwCreationFlags, lpThreadId);

    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress invalidFunctionArg
    (void) CreateThread(lpThreadAttributes, -1, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);

    // no warning shall be shown for
    return CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

// unsigned char *_mbscat(unsigned char *strDestination, const unsigned char *strSource);
unsigned char * uninitvar_mbscat(unsigned char *strDestination, const unsigned char *strSource)
{
    unsigned char *uninit_deststr;
    const unsigned char *uninit_srcstr1, *uninit_srcstr2;
    // cppcheck-suppress uninitvar
    (void)_mbscat(uninit_deststr,uninit_srcstr1);
    // cppcheck-suppress uninitvar
    (void)_mbscat(strDestination,uninit_srcstr2);

    // no warning shall be shown for
    return _mbscat(strDestination,strSource);
}

// unsigned char *_mbscat(unsigned char *strDestination, const unsigned char *strSource);
unsigned char * nullPointer_mbscat(unsigned char *strDestination, const unsigned char *strSource)
{
    // cppcheck-suppress nullPointer
    (void)_mbscat(0,strSource);
    // cppcheck-suppress nullPointer
    (void)_mbscat(strDestination,0);

    // no warning shall be shown for
    return _mbscat(strDestination,strSource);
}

// errno_t _mbscat_s(unsigned char *strDestination, size_t numberOfElements, const unsigned char *strSource );
errno_t uninitvar_mbscat_s(unsigned char *strDestination, size_t numberOfElements, const unsigned char *strSource)
{
    unsigned char *uninit_strDestination;
    size_t uninit_numberOfElements;
    const unsigned char *uninit_strSource;

    // cppcheck-suppress uninitvar
    (void)_mbscat_s(uninit_strDestination, numberOfElements, strSource);
    // cppcheck-suppress uninitvar
    (void)_mbscat_s(strDestination, uninit_numberOfElements, strSource);
    // cppcheck-suppress uninitvar
    (void)_mbscat_s(strDestination, numberOfElements, uninit_strSource);

    // no warning shall be shown for
    return _mbscat_s(strDestination, numberOfElements, strSource);
}

// errno_t _mbscat_s(unsigned char *strDestination, size_t numberOfElements, const unsigned char *strSource );
errno_t nullPointer_mbscat_s(unsigned char *strDestination, size_t numberOfElements, const unsigned char *strSource)
{
    // cppcheck-suppress nullPointer
    (void)_mbscat_s(0, numberOfElements, strSource);
    // cppcheck-suppress nullPointer
    (void)_mbscat_s(strDestination, numberOfElements, 0);

    // no warning shall be shown for
    return _mbscat_s(strDestination, numberOfElements, strSource);
}

#if !UNICODE
// errno_t _strncpy_s_l(char *strDest, size_t numberOfElements, const char *strSource, size_t count, _locale_t locale);
errno_t uninitvar__strncpy_s_l(char *strDest, size_t numberOfElements, const char *strSource, size_t count, _locale_t locale)
{
    size_t uninit_numberOfElements;
    const char *uninit_strSource;
    size_t uninit_count;
    _locale_t uninit_locale;

    // cppcheck-suppress uninitvar
    (void)_strncpy_s_l(strDest, uninit_numberOfElements, strSource, count, locale);
    // cppcheck-suppress uninitvar
    (void)_strncpy_s_l(strDest, numberOfElements, uninit_strSource, count, locale);
    // cppcheck-suppress uninitvar
    (void)_strncpy_s_l(strDest, numberOfElements, strSource, uninit_count, locale);
    // cppcheck-suppress uninitvar
    (void)_strncpy_s_l(strDest, numberOfElements, strSource, count, uninit_locale);

    // no warning shall be shown for
    return _strncpy_s_l(strDest, numberOfElements, strSource, count, locale);
}

errno_t nullPointer__strncpy_s_l(char *strDest, size_t numberOfElements, const char *strSource, size_t count, _locale_t locale)
{
    // cppcheck-suppress nullPointer
    (void)_strncpy_s_l(0, numberOfElements, strSource, count, locale);
    // cppcheck-suppress nullPointer
    (void)_strncpy_s_l(strDest, numberOfElements, 0, count, locale);

    // no warning shall be shown for
    return _strncpy_s_l(strDest, numberOfElements, strSource, count, locale);
}
#endif

void GetShortPathName_validCode(const TCHAR* lpszPath)
{
    long length = GetShortPathName(lpszPath, NULL, 0);
    if (length == 0) {
        _tprintf(TEXT("error"));
        return;
    }
    TCHAR* buffer = new TCHAR[length];
    length = GetShortPathName(lpszPath, buffer, length);
    if (length == 0) {
        delete[] buffer;
        _tprintf(TEXT("error"));
        return;
    }
    _tprintf(TEXT("long name = %s short name = %s"), lpszPath, buffer);
    delete[] buffer;
}

void invalidPrintfArgType_StructMember(double d) { // #9672
    typedef struct { CString st; } my_struct_t;

    my_struct_t my_struct;
    // cppcheck-suppress invalidPrintfArgType_sint
    my_struct.st.Format(_T("%d"), d);
}

BOOL MyEnableWindow(HWND hWnd, BOOL bEnable) {
    return EnableWindow(hWnd, bEnable);
}

int SEH_filter(unsigned int code, struct _EXCEPTION_POINTERS* ep);
int SEH_throwing_func();

void SEH_knownConditionTrueFalse() { // #8434
    int r = 0;
    __try {
        r = SEH_throwing_func();
    }
    __except (SEH_filter(GetExceptionCode(), GetExceptionInformation())) {
        r = 1;
    }
    if (r == 0) {}
}

void SEH_unusedLabel() { // #13233
    __try {
    }
    __finally {
    }
}

HWND constParameterPointer_CreateWindow(void* param) { // #14560
    return CreateWindow(L"MessageWnd", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, param);
}

void constParameterPointer_SetupDiGetDeviceInstanceId(HDEVINFO info, SP_DEVINFO_DATA *data) {
    const DWORD buffer_size = 256;
    TCHAR buffer[buffer_size];
    SetupDiGetDeviceInstanceId(info, data, buffer, buffer_size, NULL);
}
