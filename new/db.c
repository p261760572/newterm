#include<stdio.h>
#include <string.h>
#include <assert.h>

char *strcpy_s(char *dest, const char *src, size_t n) {
    assert(dest != NULL);
    assert(src != NULL);

    if(n > 0) {
        strncpy(dest,src,n-1);
        dest[n-1] = '\0';
    }
    return dest;
}