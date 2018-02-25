#include <stdlib.h>
#include <stdio.h>
#include "auxiliary.h"

void sigil_zeroize(void *a, size_t bytes)
{
    if (a == NULL || bytes <= 0)
        return;

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

int parse_number(FILE *in, size_t *number)
{
    char c;
    int digits = 0;

    *number = 0;

    // skip leading whitespaces
    while ((c = fgetc(in)) != EOF && is_whitespace(c))
        ;

    // number
    do {
        if (!is_digit(c)) {
            if (ungetc(c, in) != c)
                return 1;
            return digits == 0;
        }
        *number = 10 * *number + c - '0';
        digits++;
    } while ((c = fgetc(in)) != EOF);

    return 1;
}

int parse_free_indicator(FILE *in, char_t *result)
{
    char c;

    // skip leading whitespaces
    while ((c = fgetc(in)) != EOF && is_whitespace(c))
        ;

    switch(c) {
        case 'f':
        case 'n':
            *result = c;
            return 0;
        default:
            return 1;
    }
}

void print_module_name(const char *module_name, int verbosity)
{
    if (verbosity < 1)
        return;

    printf("\n + Testing module: %s\n", module_name);
}

void print_module_result(int result, int verbosity)
{
    if (verbosity < 1)
        return;

    if (result == 1) {
        printf(COLOR_GREEN "   PASSED\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "   FAILED\n" COLOR_RESET);
    }
}

void print_test_item(const char *test_name, int verbosity)
{
    if (verbosity < 2)
        return;

    printf("    - %-30s", test_name);
}

void print_test_result(int result, int verbosity)
{
    if (verbosity < 2)
        return;

    if (result == 1) {
        printf(COLOR_GREEN "OK\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "FAILED\n" COLOR_RESET);
    }
}

int sigil_auxiliary_self_test(int verbosity)
{
    print_module_name("auxiliary", verbosity);

    // TEST: MIN and MAX macros
    print_test_item("MIN, MAX", verbosity);

    if (MIN(MAX(1, 2), MIN(3, 4)) != 2 ||
        MAX(MAX(1, 2), MIN(3, 4)) != 3)
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    // TEST: fn sigil_zeroize
    print_test_item("fn sigil_zeroize", verbosity);

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
        goto failed;
    }

    print_test_result(1, verbosity);

    // TEST: fn is_digit
    print_test_item("fn is_digit", verbosity);

    if ( is_digit('/') ||
        !is_digit('0') ||
        !is_digit('3') ||
        !is_digit('9') ||
         is_digit(':') )
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    // TEST: fn is_whitespace
    print_test_item("fn is_whitespace", verbosity);

    if (!is_whitespace(0x00) || is_whitespace(0x01) ||
        !is_whitespace(0x09) || is_whitespace(0x08) ||
        !is_whitespace(0x0a) || is_whitespace(0x0b) ||
        !is_whitespace(0x0c) || is_whitespace('a' ) ||
        !is_whitespace(0x0d) || is_whitespace('!' ) ||
        !is_whitespace(0x20) || is_whitespace('_' ) )
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    print_test_result(0, verbosity);
    print_module_result(0, verbosity);
    return 1;
}
