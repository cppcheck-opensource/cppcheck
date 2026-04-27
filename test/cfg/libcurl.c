
// Test library configuration for libcurl.cfg
//
// Usage:
// $ cppcheck --check-library --library=libcurl --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/libcurl.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

// cppcheck-suppress-file valueFlowBailout

#include <curl/curl.h>
#include <stdio.h>

struct S_WriteHeader {
    int x;
};

size_t cb_WriteHeader(void * /*ptr*/, size_t size, size_t nmemb, void *p) {
    struct S_WriteHeader *s = (struct S_WriteHeader *)p;
    return s->x ? size * nmemb : 0;
}

void validCode()
{
    CURL *curl = curl_easy_init();
    if (curl) {
        CURLcode res;
        // cppcheck-suppress valueFlowBailoutIncompleteVar
        curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, cb_WriteHeader);
        struct S_WriteHeader s;
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &s); // #14692
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("error");
        } else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            printf("%ld", response_code);
            char * pStr = curl_easy_escape(curl, "a", 1);
            if (pStr)
                printf("%s", pStr);
            curl_free(pStr);
            curl_easy_reset(curl);
        }
        curl_easy_cleanup(curl);
    }
}

// cppcheck-suppress constParameterPointer
void ignoredReturnValue(CURL * handle)
{
    // cppcheck-suppress ignoredReturnValue
    curl_easy_strerror(1);
}

void resourceLeak_curl_easy_init()
{
    const CURL *curl = curl_easy_init();
    printf("%p", curl);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_curl_easy_duphandle(CURL * handle)
{
    const CURL *curl = curl_easy_duphandle(handle);
    printf("%p", curl);
    // cppcheck-suppress resourceLeak
}

void memleak_curl_easy_escape(CURL * handle)
{
    const char * pStr = curl_easy_escape(handle, "a", 1);
    if (pStr)
        printf("%s", pStr);
    // cppcheck-suppress memleak
}

void nullPointer(CURL * handle)
{
    char * buf[10] = {0};
    size_t len;

    curl_easy_recv(handle, buf, 10, &len);
    // cppcheck-suppress nullPointer
    curl_easy_recv(handle, buf, 10, NULL);
    curl_easy_send(handle, buf, 10, &len);
    // cppcheck-suppress nullPointer
    curl_easy_send(handle, buf, 10, NULL);
}

void uninitvar(CURL * handle)
{
    const char * bufInit[10] = {0};
    const char * bufUninit;
    size_t len;

    curl_easy_send(handle, bufInit, 10, &len);
    // cppcheck-suppress uninitvar
    curl_easy_send(handle, bufUninit, 10, &len);
}
