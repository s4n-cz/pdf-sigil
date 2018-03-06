#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "constants.h"
#include "sigil.h"


void sigil_zeroize(void *a, size_t bytes)
{
    if (a == NULL || bytes <= 0)
        return;

    volatile char *p = a;

    while (bytes--) {
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
            c == 0x20);  // space
}

// writes size bytes of data + 1 byte of null terminator to result
// res_size is just the number of read bytes, without the terminating null
sigil_err_t pdf_read(sigil_t *sgl, size_t size, char *result, size_t *res_size)
{
    size_t read_size;
    size_t items_processed;

    if (sgl == NULL || size <= 0 || result == NULL || res_size == NULL)
        return ERR_PARAMETER;

    if (sgl->pdf_data.buffer != NULL) {
        read_size = MIN(size, sgl->pdf_data.size - sgl->pdf_data.buf_pos);
        if (read_size <= 0)
            return ERR_NO_DATA;

        if (memcpy(result, &(sgl->pdf_data.buffer[sgl->pdf_data.buf_pos]),
            read_size) != result)
        {
            return ERR_IO;
        }
        result[read_size] = '\0';
        sgl->pdf_data.buf_pos += read_size;

        *res_size = read_size;

        return ERR_NO;
    }

    if (sgl->pdf_data.file != NULL) {
        items_processed = fread(result, size, sizeof(char), sgl->pdf_data.file);
        if (items_processed <= 0 || items_processed > size)
            return ERR_IO;
        read_size = items_processed * sizeof(char);
        result[read_size] = '\0';

        *res_size = read_size;

        return ERR_NO;
    }

    return ERR_NO_DATA;
}

sigil_err_t pdf_get_char(sigil_t *sgl, char *result)
{
    if (sgl == NULL || result == NULL)
        return ERR_PARAMETER;

    if (sgl->pdf_data.buffer != NULL) {
        if (sgl->pdf_data.buf_pos >= sgl->pdf_data.size)
            return ERR_NO_DATA;

        *result = sgl->pdf_data.buffer[(sgl->pdf_data.buf_pos)++];
        return ERR_NO;
    }

    if (sgl->pdf_data.file != NULL) {
        *result = getc(sgl->pdf_data.file);
        if (*result == EOF)
            return ERR_NO_DATA;
        return ERR_NO;
    }

    return ERR_NO_DATA;
}

sigil_err_t pdf_peek_char(sigil_t *sgl, char *result)
{
    sigil_err_t err;

    err = pdf_get_char(sgl, result);
    if (err != ERR_NO)
        return err;

    if (sgl->pdf_data.buffer != NULL) {
        if (--(sgl->pdf_data.buf_pos) < 0)
            return ERR_IO;
        return ERR_NO;
    }

    if (sgl->pdf_data.file != NULL) {
        if (ungetc(*result, sgl->pdf_data.file) != *result)
            return ERR_IO;
        return ERR_NO;
    }

    return ERR_NO_DATA;
}

// shifts position relatively to current position in boundaries between
//   beginning of file and end of file
sigil_err_t pdf_move_pos_rel(sigil_t *sgl, ssize_t shift_bytes)
{
    ssize_t final_position;

    if (sgl == NULL)
        return ERR_PARAMETER;

    if (shift_bytes == 0)
        return ERR_NO;

    if (sgl->pdf_data.buffer != NULL) {
        final_position = sgl->pdf_data.buf_pos + shift_bytes;
        if (final_position < 0) {
            final_position = 0;
        } else if (final_position > sgl->pdf_data.size - 1) {
            final_position = sgl->pdf_data.size - 1;
        }

        sgl->pdf_data.buf_pos = final_position;

        return ERR_NO;
    }

    if (sgl->pdf_data.file != NULL) {
        if (fseek(sgl->pdf_data.file, shift_bytes, SEEK_CUR) != 0)
            return ERR_IO;
        return ERR_NO;
    }

    return ERR_NO_DATA;
}

// shifts position to absolute position in file
sigil_err_t pdf_move_pos_abs(sigil_t *sgl, size_t position)
{
    if (sgl == NULL)
        return ERR_PARAMETER;

    if (position == 0)
        return ERR_NO;

    if (sgl->pdf_data.buffer != NULL) {
        if (position > sgl->pdf_data.size - 1)
            return ERR_IO;

        sgl->pdf_data.buf_pos = position;

        return ERR_NO;
    }

    if (sgl->pdf_data.file != NULL) {
        if (fseek(sgl->pdf_data.file, position, SEEK_SET) != 0)
            return ERR_IO;

        return ERR_NO;
    }

    return ERR_NO_DATA;
}

sigil_err_t skip_leading_whitespaces(sigil_t *sgl)
{
    sigil_err_t err;
    char c;

    while((err = pdf_peek_char(sgl, &c)) == ERR_NO) {
        if (!is_whitespace(c))
            return ERR_NO;

        err = pdf_move_pos_rel(sgl, 1);
        if (err != ERR_NO)
            return err;
    }

    return err;
}

// without leading "<<"
sigil_err_t skip_dictionary(sigil_t *sgl)
{
    sigil_err_t err;
    char c;

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NO)
        return err;

    while ((err = pdf_get_char(sgl, &c)) == ERR_NO) {
        switch (c) {
            case '>':
                if ((err = pdf_get_char(sgl, &c)) != ERR_NO)
                    return err;
                if (c == '>')
                    return ERR_NO;
                break;
            case '<':
                if ((err = pdf_get_char(sgl, &c)) != ERR_NO)
                    return err;
                if (c != '<')
                    break;
                if ((err = skip_dictionary(sgl)) != ERR_NO)
                    return err;
                break;
            default:
                break;
        }
    }

    return err;
}

sigil_err_t skip_dict_unknown_value(sigil_t *sgl)
{
    sigil_err_t err;
    char c;

    while ((err = pdf_peek_char(sgl, &c)) == ERR_NO) {
        switch (c) {
            case '/':
                return ERR_NO;
            case '<':
                if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NO)
                    return err;
                if ((err = pdf_peek_char(sgl, &c)) != ERR_NO)
                    return err;
                if (c != '<')
                    break;
                if ((err = skip_dictionary(sgl)) != ERR_NO)
                    return err;
                return ERR_NO;
            default:
                break;
        }
    }

    return err;
}

sigil_err_t parse_number(sigil_t *sgl, size_t *number)
{
    sigil_err_t err;
    char c;
    int digits = 0;

    *number = 0;

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NO)
        return err;

    // number
    while ((err = pdf_peek_char(sgl, &c)) == ERR_NO) {
        if (!is_digit(c)) {
            if (digits > 0) {
                return ERR_NO;
            } else {
                return ERR_PDF_CONTENT;
            }
        }

        *number = 10 * (*number) + c - '0';

        if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NO)
            return err;

        digits++;
    }

    return err;
}

sigil_err_t parse_keyword(sigil_t *sgl, keyword_t *keyword)
{
    sigil_err_t err;
    int count = 0;
    const int keyword_max = 10;
    char tmp[keyword_max],
         c;

    sigil_zeroize(tmp, keyword_max * sizeof(*tmp));

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NO)
        return err;

    while ((err = pdf_peek_char(sgl, &c)) == ERR_NO) {
        if (is_whitespace(c)) {
            if (count <= 0)
                return ERR_PDF_CONTENT;
            break;
        } else {
            if (count >= keyword_max - 1)
                return ERR_PDF_CONTENT;
            tmp[count++] = c;
        }

        if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NO)
            return err;
    }

    if (err != ERR_NO)
        return err;

    if (strncmp(tmp, "xref", 4) == 0) {
        *keyword = KEYWORD_xref;
        return ERR_NO;
    }
    if (strncmp(tmp, "trailer", 7) == 0) {
        *keyword = KEYWORD_trailer;
        return ERR_NO;
    }

    return ERR_PDF_CONTENT;
}

sigil_err_t parse_free_indicator(sigil_t *sgl, free_indicator_t *result)
{
    sigil_err_t err;
    char c;

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NO)
        return err;

    err = pdf_get_char(sgl, &c);
    if (err != ERR_NO)
        return err;

    switch(c) {
        case 'f':
            *result = FREE_ENTRY;
            return ERR_NO;
        case 'n':
            *result = IN_USE_ENTRY;
            return ERR_NO;
        default:
            return ERR_PDF_CONTENT;
    }
}

sigil_err_t parse_indirect_reference(sigil_t *sgl, reference_t *ref)
{
    sigil_err_t err;
    char c;

    err = parse_number(sgl, &ref->object_num);
    if (err != ERR_NO)
        return err;

    err = parse_number(sgl, &ref->generation_num);
    if (err != ERR_NO)
        return err;

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NO)
        return err;

    err = pdf_get_char(sgl, &c);
    if (err != ERR_NO)
        return err;

    if (c != 'R')
        return ERR_PDF_CONTENT;

    return ERR_NO;
}

// parse the key of the couple key - value in the dictionary
sigil_err_t parse_dict_key(sigil_t *sgl, dict_key_t *dict_key)
{
    sigil_err_t err;
    const int dict_key_max = 10;
    int count = 0;
    char tmp[dict_key_max],
         c;

    sigil_zeroize(tmp, dict_key_max * sizeof(*tmp));

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NO)
        return err;

    err = pdf_peek_char(sgl, &c);
    if (err != ERR_NO)
        return err;

    switch (c) {
        case '/':
            break;
        case '>': // test end of dictionary
            if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NO)
                return err;
            if ((err = pdf_get_char(sgl, &c)) != ERR_NO)
                return err;
            if (c != '>')
                return ERR_PDF_CONTENT;
            return ERR_END_OF_DICT;
        default:
            return ERR_PDF_CONTENT;
    }


    while ((err = pdf_peek_char(sgl, &c)) == ERR_NO) {
        if (is_whitespace(c)) {
            if (count <= 0)
                return ERR_PDF_CONTENT;
            break;
        } else {
            if (count >= dict_key_max - 1)
                return ERR_PDF_CONTENT;
            tmp[count++] = c;
        }

        if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NO)
            return err;
    }

    if (strncmp(tmp, "Size", 4) == 0) {
        *dict_key = DICT_KEY_Size;
    } else if (strncmp(tmp, "Prev", 4) == 0) {
        *dict_key = DICT_KEY_Prev;
    } else if (strncmp(tmp, "Root", 4) == 0) {
        *dict_key = DICT_KEY_Root;
    } else {
        *dict_key = DICT_KEY_UNKNOWN;
    }

    return ERR_NO;
}

const char *sigil_err_string(sigil_err_t err)
{
    switch (err) {
        case ERR_NO:
            return "finished without any error";
        case ERR_ALLOCATION:
            return "ERROR during allocation";
        case ERR_PARAMETER:
            return "ERROR bad data between function parameters";
        case ERR_IO:
            return "ERROR during performing input/output operation";
        case ERR_PDF_CONTENT:
            return "ERROR unexpected data on input, probably corrupted PDF file";
        case ERR_NOT_IMPLEMENTED:
            return "ERROR this functionality is not currently available";
        case ERR_NO_DATA:
            return "ERROR no data available";
        case ERR_END_OF_DICT:
            return "ERROR end of dictionary occured while processing it's content";
        default:
            return "ERROR unknown";
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

sigil_t *test_prepare_sgl_content(char *content, size_t size)
{
    sigil_t *sgl;

    if (sigil_init(&sgl) != ERR_NO)
        return NULL;

    if (sigil_set_pdf_buffer(sgl, content, size) != ERR_NO) {
        sigil_free(&sgl);
        return NULL;
    }

    return sgl;
}

sigil_t *test_prepare_sgl_path(const char *path)
{
    sigil_t *sgl;

    if (sigil_init(&sgl) != ERR_NO)
        return NULL;

    if (sigil_set_pdf_path(sgl, path) != ERR_NO) {
        sigil_free(&sgl);
        return NULL;
    }

    return sgl;
}

int sigil_auxiliary_self_test(int verbosity)
{
    sigil_t *sgl = NULL;
    char c;

    print_module_name("auxiliary", verbosity);

    // TEST: UTF-8 filepath support
    print_test_item("UTF-8 filepath support", verbosity);

    {
        if ((sgl = test_prepare_sgl_path("test/utf-8_test_€䪜勁𠹹")) == NULL)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NO || c != 'x')
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

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

    char array[5];
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
        if ((sgl = test_prepare_sgl_content("x", 2)) == NULL)
            goto failed;

        if (skip_leading_whitespaces(sgl) != ERR_NO)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NO || c != 'x')
            goto failed;

        sigil_free(&sgl);

        if ((sgl = test_prepare_sgl_content("\x00\x09\x0a\x0c\x0d\x20x", 8)) == NULL)
            goto failed;

        if (skip_leading_whitespaces(sgl) != ERR_NO)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NO || c != 'x')
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn skip_dictionary
    print_test_item("fn skip_dictionary", verbosity);

    {
        char *sstream = "/First 2 0 R\n"        \
                        "/Second 37 "           \
                        "/Third true "          \
                        "/Fourth [<86C><BA3>] " \
                        ">>x";
        if ((sgl = test_prepare_sgl_content(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (skip_dictionary(sgl) != ERR_NO)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NO || c != 'x')
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn skip_dict_unknown_value
    print_test_item("fn skip_dict_unknown_value", verbosity);

    {
        char *sstream = "<</First /NameVal\n"    \
                        "/Second <</Nested -32 " \
                        ">> >>x";
        if ((sgl = test_prepare_sgl_content(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (skip_dict_unknown_value(sgl) != ERR_NO)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NO || c != 'x')
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn parse_number
    print_test_item("fn parse_number", verbosity);

    {
        size_t result = 0;

        char *sstream = "0123456789    42";
        if ((sgl = test_prepare_sgl_content(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (parse_number(sgl, &result) != ERR_NO || result != 123456789)
            goto failed;

        if (parse_number(sgl, &result) != ERR_NO || result != 42)
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn parse_keyword
    print_test_item("fn parse_keyword", verbosity);

    {
        keyword_t result;

        char *sstream = " xref \n trailer";
        if ((sgl = test_prepare_sgl_content(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (parse_keyword(sgl, &result) != ERR_NO || result != KEYWORD_xref)
            goto failed;

        if (parse_keyword(sgl, &result) != ERR_NO || result != KEYWORD_trailer)
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn parse_free_indicator
    print_test_item("fn parse_free_indicator", verbosity);

    {
        free_indicator_t result;

        char *sstream = " f n";
        if ((sgl = test_prepare_sgl_content(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (parse_free_indicator(sgl, &result) != ERR_NO || result != FREE_ENTRY)
            goto failed;

        if (parse_free_indicator(sgl, &result) != ERR_NO || result != IN_USE_ENTRY)
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    if (sgl)
        sigil_free(&sgl);

    print_test_result(0, verbosity);
    print_module_result(0, verbosity);

    return 1;
}
