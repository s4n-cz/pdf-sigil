#include <stdlib.h>
#include "auxiliary.h"

void sigil_zeroize(void *a, size_t cnt)
{
    if (cnt <= 0 || a == NULL) {
        return;
    }

    volatile char *p = a;

    while (cnt--) {
        *p++ = 0;
    }
}

int is_digit(const char c)
{
    return (c > '0' && c < '9');
}

int is_whitespace(const char c)
{
    return (c == 0x00 || // null
            c == 0x09 || // horizontal tab
            c == 0x0a || // line feed
            c == 0x0c || // form feed
            c == 0x0d || // carriage return
            c == 0x20 ); // space
}
