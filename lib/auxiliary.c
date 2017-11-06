#include <stdlib.h>
#include <stdio.h>
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
    return (c >= '0' && c <= '9');
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

int sigil_auxiliary_self_test(int quiet)
{
    if (!quiet)
        printf("\n + Testing module: auxiliary\n");

    // TEST: MIN and MAX macros
    if (!quiet)
        printf("    - %-30s", "MIN, MAX");

    if (MIN(1, 2) != 1 ||
        MAX(1, 2) != 2 )
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: fn sigil_zeroize
    if (!quiet)
        printf("    - %-30s", "fn sigil_zeroize");

    char array[5];
    for (int i = 0; i < 5; i++) {
        array[i] = 1;
    }

    sigil_zeroize(array + 1, 3);

    if (array[0] != 1 ||
        array[1] != 0 ||
        array[2] != 0 ||
        array[3] != 0 ||
        array[4] != 1 )
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: fn is_digit
    if (!quiet)
        printf("    - %-30s", "fn is_digit");

    if ( is_digit('/') ||
        !is_digit('0') ||
        !is_digit('3') ||
        !is_digit('9') ||
         is_digit(':') )
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: fn is_whitespace
    if (!quiet)
        printf("    - %-30s", "fn is_whitespace");

    if (!is_whitespace(0x09) ||
        !is_whitespace(0x20) ||
         is_whitespace('a')  )
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // all tests done
    if (!quiet) {
        printf("   PASSED\n");
        fflush(stdout);
    }

    return 0;

failed:
    if (!quiet) {
        printf("   FAILED\n");
        fflush(stdout);
    }

    return 1;

}
