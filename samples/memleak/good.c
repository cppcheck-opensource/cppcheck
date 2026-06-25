#include <stdlib.h>
int main()
{
    int result = 0;
    char *a = malloc(10);
    if (a) {
        a[0] = 0;
        result = a[0];
        free(a);
    }
    return result;
}
