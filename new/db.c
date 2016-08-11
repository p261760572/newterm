#include<stdio.h>
#include <string.h>
#include <assert.h>

char *strcpy_s(char *dest, const char *src, size_t n) {
    if(dest == NULL) {
        return dest;
    }

    if(n == 0) { //error
    } else if(src == NULL) {
        dest[0] = '\0';
    } else {
        strncpy(dest,src,n-1);
        dest[n-1] = '\0';
    }

    return dest;
}