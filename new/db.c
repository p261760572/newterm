#include<stdio.h>
#include <string.h>
#include <assert.h>

char *strcpy_s(char *dest, const char *src, size_t n) {
    if(dest == NULL) {
        return dest;
    }

	dest[0] = '\0';

	if(src != NULL && n > 0) {
        strncpy(dest,src,n);
        dest[n-1] = '\0';
    }

    return dest;
}