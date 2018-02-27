#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "error.h"

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

sigil_err_t skip_leading_whitespaces(FILE *in)
{
    char_t c;

    while ((c = fgetc(in)) != EOF && is_whitespace(c))
        ;

    if (c == EOF)
        return (sigil_err_t)ERR_PDF_CONT;

    if (ungetc(c, in) != c)
        return (sigil_err_t)ERR_IO;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t skip_dictionary(FILE *in)
{
    sigil_err_t err;
    char c;

    err = skip_leading_whitespaces(in);
    if (err != ERR_NO)
        return err;

    while ((c = fgetc(in)) != EOF) {
        switch (c) {
            case '>':
                if (fgetc(in) == '>')
                    return (sigil_err_t)ERR_NO;
                break;
            case '<':
                if (fgetc(in) != '<')
                    break;
                if ((err = skip_dictionary(in)) != ERR_NO)
                    return err;
                return (sigil_err_t)ERR_NO;
            default:
                break;
        }
    }

    return (sigil_err_t)ERR_PDF_CONT;
}

sigil_err_t skip_dict_unknown_value(FILE *in)
{
    sigil_err_t err;
    char c;

    while ((c = fgetc(in)) != EOF) {
        switch (c) {
            case '/':
                if (ungetc(c, in) != c)
                    return (sigil_err_t)ERR_IO;
                return (sigil_err_t)ERR_NO;
            case '<':
                if (fgetc(in) != '<')
                    break;
                if ((err = skip_dictionary(in)) != ERR_NO)
                    return err;
                return (sigil_err_t)ERR_NO;
            default:
                break;
        }
    }

    return (sigil_err_t)ERR_PDF_CONT;
}

sigil_err_t parse_number(FILE *in, size_t *number)
{
    char_t c;
    int digits = 0;
    sigil_err_t err;

    *number = 0;

    err = skip_leading_whitespaces(in);
    if (err != ERR_NO)
        return err;

    // number
    while ((c = fgetc(in)) != EOF) {
        if (!is_digit(c)) {
            if (ungetc(c, in) != c)
                return (sigil_err_t)ERR_IO;
            if (digits > 0) {
                return (sigil_err_t)ERR_NO;
            } else {
                return (sigil_err_t)ERR_PDF_CONT;
            }
        }
        *number = 10 * *number + c - '0';
        digits++;
    }

    return (sigil_err_t)ERR_PDF_CONT;
}

sigil_err_t parse_keyword(FILE *in, keyword_t *keyword)
{
    sigil_err_t err;
    int count = 0;
    const int keyword_max = 10;
    char tmp[keyword_max],
         c;

    sigil_zeroize(tmp, keyword_max * sizeof(*tmp));

    err = skip_leading_whitespaces(in);
    if (err != ERR_NO)
        return err;

    while ((c = fgetc(in)) != EOF) {
        if (is_whitespace(c)) {
            if (count <= 0)
                return (sigil_err_t)ERR_PDF_CONT;
            if (ungetc(c, in) != c)
                return (sigil_err_t)ERR_IO;

            if (strncmp(tmp, "xref", 4) == 0) {
                *keyword = KEYWORD_xref;
                return (sigil_err_t)ERR_NO;
            }
            if (strncmp(tmp, "trailer", 7) == 0) {
                *keyword = KEYWORD_trailer;
                return (sigil_err_t)ERR_NO;
            }
            return (sigil_err_t)ERR_PDF_CONT;
        } else {
            if (count >= keyword_max - 1)
                return (sigil_err_t)ERR_PDF_CONT;
            tmp[count] = c;
            count++;
        }
    }

    return (sigil_err_t)ERR_PDF_CONT;
}

sigil_err_t parse_free_indicator(FILE *in, free_indicator_t *result)
{
    sigil_err_t err;
    char c;

    err = skip_leading_whitespaces(in);
    if (err != ERR_NO)
        return err;

    c = fgetc(in);

    switch(c) {
        case 'f':
            *result = FREE_ENTRY;
            return (sigil_err_t)ERR_NO;
        case 'n':
            *result = IN_USE_ENTRY;
            return (sigil_err_t)ERR_NO;
        default:
            return (sigil_err_t)ERR_PDF_CONT;
    }
}

sigil_err_t parse_indirect_reference(FILE *in, reference_t *ref)
{
    sigil_err_t err;

    err = parse_number(in, &ref->object_num);
    if (err != ERR_NO)
        return err;
    err = parse_number(in, &ref->generation_num);
    if (err != ERR_NO)
        return err;
    err = skip_leading_whitespaces(in);
    if (err != ERR_NO)
        return err;
    if (fgetc(in) != 'R')
        return (sigil_err_t)ERR_PDF_CONT;
    return (sigil_err_t)ERR_NO;
}

// parse the key of the couple key - value in the dictionary
sigil_err_t parse_dict_key(FILE *in, dict_key_t *dict_key)
{
    sigil_err_t err;
    const int dict_key_max = 10;
    int count = 0;
    char tmp[dict_key_max],
         c;

    sigil_zeroize(tmp, dict_key_max * sizeof(*tmp));

    err = skip_leading_whitespaces(in);
    if (err != ERR_NO)
        return err;

    if ((c = fgetc(in)) == EOF)
        return 1;

    switch (c) {
        case '/':
            break;
        case '>':
            if ((c = fgetc(in)) == '>')
                return (sigil_err_t)ERR_PDF_CONT;
            if (ungetc(c, in) != c)
                return (sigil_err_t)ERR_IO;
            return (sigil_err_t)ERR_PDF_CONT;
        default:
            return (sigil_err_t)ERR_PDF_CONT;
    }

    while ((c = fgetc(in)) != EOF) {
        if (is_whitespace(c)) {
            if (count <= 0)
                return (sigil_err_t)ERR_PDF_CONT;
            if (ungetc(c, in) != c)
                return (sigil_err_t)ERR_IO;

            if (strncmp(tmp, "Size", 4) == 0) {
                *dict_key = DICT_KEY_Size;
                return (sigil_err_t)ERR_NO;
            }
            if (strncmp(tmp, "Prev", 4) == 0) {
                *dict_key = DICT_KEY_Prev;
                return (sigil_err_t)ERR_NO;
            }
            if (strncmp(tmp, "Root", 4) == 0) {
                *dict_key = DICT_KEY_Root;
                return (sigil_err_t)ERR_NO;
            }
            *dict_key = DICT_KEY_unknown;
            return (sigil_err_t)ERR_NO;
        } else {
            if (count >= dict_key_max - 1)
                return (sigil_err_t)ERR_ALLOC;
            tmp[count] = c;
            count++;
        }
    }

    return (sigil_err_t)ERR_PDF_CONT;
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

    printf("    - %-32s", test_name);
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

    // TEST: fn skip_leading_whitespaces
    print_test_item("fn skip_leading_whitespaces", verbosity);

    {
        char *sstream_1 = "x";
        char *sstream_2 = "\x00\x09\x0a\x0c\x0d\x20x";
        FILE *file;

        file = fmemopen(sstream_1,
                        (strlen(sstream_1) + 1) * sizeof(*sstream_1),
                        "r");
        if (file == NULL)
            goto failed;

        if (skip_leading_whitespaces(file) != ERR_NO ||
            fgetc(file) != 'x')
        {
            goto failed;
        }

        fclose(file);

        file = fmemopen(sstream_2,
                        (7 + 1) * sizeof(*sstream_2), // cannot use strlen
                        "r");
        if (file == NULL)
            goto failed;

        if (skip_leading_whitespaces(file) != ERR_NO ||
            fgetc(file) != 'x')
        {
            goto failed;
        }

        fclose(file);
    }

    print_test_result(1, verbosity);

    // TEST: fn parse_number
    print_test_item("fn parse_number", verbosity);

    {
        char *sstream = "0123456789    42";
        size_t result = 0;
        FILE *file;

        file = fmemopen(sstream,
                        (strlen(sstream) + 1) * sizeof(*sstream),
                        "r");
        if (file == NULL)
            goto failed;

        if (parse_number(file, &result) != ERR_NO ||
            result != 123456789)
        {
            goto failed;
        }

        if (parse_number(file, &result) != ERR_NO ||
            result != 42)
        {
            goto failed;
        }

        fclose(file);
    }

    print_test_result(1, verbosity);

    // TEST: fn parse_keyword
    print_test_item("fn parse_keyword", verbosity);

    {
        char *sstream = " xref \n trailer";
        keyword_t result;
        FILE *file;

        file = fmemopen(sstream,
                        (strlen(sstream) + 1) * sizeof(*sstream),
                        "r");
        if (file == NULL)
            goto failed;

        if (parse_keyword(file, &result) != ERR_NO ||
            result != KEYWORD_xref)
        {
            goto failed;
        }

        if (parse_keyword(file, &result) != ERR_NO ||
            result != KEYWORD_trailer)
        {
            goto failed;
        }

        fclose(file);
    }

    print_test_result(1, verbosity);

    // TEST: fn parse_free_indicator
    print_test_item("fn parse_free_indicator", verbosity);

    {
        char *sstream = " f n";
        free_indicator_t result;
        FILE *file;

        file = fmemopen(sstream,
                        (strlen(sstream) + 1) * sizeof(*sstream),
                        "r");
        if (file == NULL)
            goto failed;

        if (parse_free_indicator(file, &result) != ERR_NO ||
            result != FREE_ENTRY)
        {
            goto failed;
        }

        if (parse_free_indicator(file, &result) != ERR_NO ||
            result != IN_USE_ENTRY)
        {
            goto failed;
        }

        fclose(file);
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
