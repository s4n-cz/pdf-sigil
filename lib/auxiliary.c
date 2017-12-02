#include <stdlib.h>
#include <stdio.h>
#include "auxiliary.h"

void sigil_zeroize(void *a, size_t bytes)
{
    if (a == NULL || bytes <= 0) {
        return;
    }

    volatile char *p = a;

    while (bytes--) {
        *p++ = 0;
    }
}

int is_digit(const char_t c)
{
    return (c >= '0' && c <= '9');
}

int is_whitespace(const char_t c)
{
    return (c == 0x00 || // null
            c == 0x09 || // horizontal tab
            c == 0x0a || // line feed
            c == 0x0c || // form feed
            c == 0x0d || // carriage return
            c == 0x20);  // space
}

void v_print(const char *txt, int align, int verbosity, int threshold)
{
    if (verbosity >= threshold) {
        printf("%*s", align, txt);
    }
}

int sigil_auxiliary_self_test(int verbosity)
{
    v_print("\n + Testing module: auxiliary\n", 0, verbosity, 1);

    // TEST: MIN and MAX macros
    v_print("    - MIN, MAX", -35, verbosity, 2);

    if (MIN(MAX(1, 2), MIN(3, 4)) != 2 ||
        MAX(MAX(1, 2), MIN(3, 4)) != 3)
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // TEST: fn sigil_zeroize
    v_print("    - fn sigil_zeroize", -35, verbosity, 2);

    char_t array[5];
    for (int i = 0; i < 5; i++) {
        array[i] = 1;
    }

    sigil_zeroize(array + 1, 3 * sizeof(*array));

    if (array[0] != 1 ||
        array[1] != 0 ||
        array[2] != 0 ||
        array[3] != 0 ||
        array[4] != 1 )
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // TEST: fn is_digit
    v_print("    - fn is_digit", -35, verbosity, 2);

    if ( is_digit('/') ||
        !is_digit('0') ||
        !is_digit('3') ||
        !is_digit('9') ||
         is_digit(':') )
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // TEST: fn is_whitespace
    v_print("    - fn is_whitespace", -35, verbosity, 2);

    if (!is_whitespace(0x09) ||
        !is_whitespace(0x20) ||
         is_whitespace('a')  )
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // all tests done
    v_print(COLOR_GREEN "   PASSED\n" COLOR_RESET, 0, verbosity, 1);
    return 0;

failed:
    v_print(COLOR_RED "   FAILED\n" COLOR_RESET, 0, verbosity, 1);
    return 1;
}
