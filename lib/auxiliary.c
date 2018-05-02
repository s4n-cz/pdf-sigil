#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "auxiliary.h"
#include "config.h"
#include "constants.h"
#include "sigil.h"
#include "types.h"

#define DICT_KEY_MAX   20


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

sigil_err_t pdf_read(sigil_t *sgl, size_t size, char *result, size_t *res_size)
{
    size_t read_size;
    size_t processed,
           total_processed;

    if (sgl == NULL || size == 0 || result == NULL || res_size == NULL)
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

        return ERR_NONE;
    }

    if (sgl->pdf_data.file != NULL) {
        total_processed = 0;

        while (total_processed < size) {
            processed = fread(result + total_processed, sizeof(char), size,
                              sgl->pdf_data.file);
            total_processed += processed;
            if (processed <= 0 || total_processed * sizeof(char) > size)
                return ERR_IO;
        }
        result[total_processed] = '\0';

        *res_size = total_processed;

        return ERR_NONE;
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
        return ERR_NONE;
    }

    if (sgl->pdf_data.file != NULL) {
        *result = (char)getc(sgl->pdf_data.file);
        if (*result == EOF)
            return ERR_NO_DATA;
        return ERR_NONE;
    }

    return ERR_NO_DATA;
}

sigil_err_t pdf_peek_char(sigil_t *sgl, char *result)
{
    sigil_err_t err;

    if (sgl == NULL || result == NULL)
        return ERR_PARAMETER;

    err = pdf_get_char(sgl, result);
    if (err != ERR_NONE)
        return err;

    if (sgl->pdf_data.buffer != NULL) {
        if (--(sgl->pdf_data.buf_pos) < 0)
            return ERR_IO;
        return ERR_NONE;
    }

    if (sgl->pdf_data.file != NULL) {
        if (ungetc(*result, sgl->pdf_data.file) != *result)
            return ERR_IO;
        return ERR_NONE;
    }

    return ERR_NO_DATA;
}

sigil_err_t pdf_move_pos_rel(sigil_t *sgl, ssize_t shift_bytes)
{
    ssize_t final_position;

    if (sgl == NULL)
        return ERR_PARAMETER;

    if (shift_bytes == 0)
        return ERR_NONE;

    if (sgl->pdf_data.buffer != NULL) {
        final_position = sgl->pdf_data.buf_pos + shift_bytes;
        if (final_position < sgl->offset_pdf_start) {
            final_position = sgl->offset_pdf_start;
        } else if ((size_t)final_position > sgl->pdf_data.size - 1) {
            final_position = sgl->pdf_data.size - 1;
        }

        sgl->pdf_data.buf_pos = (size_t)final_position;

        return ERR_NONE;
    }

    if (sgl->pdf_data.file != NULL) {
        if (fseek(sgl->pdf_data.file, shift_bytes, SEEK_CUR) != 0)
            return ERR_IO;
        return ERR_NONE;
    }

    return ERR_NO_DATA;
}

// shifts position to absolute position in file
sigil_err_t pdf_move_pos_abs(sigil_t *sgl, size_t position)
{
    size_t final_position;

    if (sgl == NULL)
        return ERR_PARAMETER;

    final_position = position + sgl->offset_pdf_start;

    if (sgl->pdf_data.buffer != NULL) {
        if (final_position > sgl->pdf_data.size - 1)
            return ERR_IO;

        sgl->pdf_data.buf_pos = final_position;

        return ERR_NONE;
    }

    if (sgl->pdf_data.file != NULL) {
        if (fseek(sgl->pdf_data.file, final_position, SEEK_SET) != 0)
            return ERR_IO;

        return ERR_NONE;
    }

    return ERR_NO_DATA;
}

sigil_err_t pdf_goto_obj(sigil_t *sgl, reference_t *ref)
{
    sigil_err_t err;
    size_t offset,
           tmp;

    if (sgl == NULL || ref == NULL)
        return ERR_PARAMETER;

    if (ref->object_num <= 0 &&
        ref->generation_num <= 0)
    {
        return ERR_NO_DATA;
    }

    err = reference_to_offset(sgl, ref, &offset);
    if (err != ERR_NONE)
        return err;

    err = pdf_move_pos_abs(sgl, offset);
    if (err != ERR_NONE)
        return err;

    err = parse_number(sgl, &tmp);
    if (err != ERR_NONE)
        return err;
    if (tmp != ref->object_num)
        return ERR_PDF_CONTENT;

    err = parse_number(sgl, &tmp);
    if (err != ERR_NONE)
        return err;
    if (tmp != ref->generation_num)
        return ERR_PDF_CONTENT;

    err = skip_word(sgl, "obj");
    if (err != ERR_NONE)
        return err;

    return ERR_NONE;
}

sigil_err_t get_curr_position(sigil_t *sgl, size_t *result)
{
    long file_position;

    if (sgl == NULL || result == NULL)
        return ERR_PARAMETER;

    if (sgl->pdf_data.buffer != NULL) {
        if (sgl->offset_pdf_start > sgl->pdf_data.buf_pos)
            return ERR_IO;

        *result = sgl->pdf_data.buf_pos - sgl->offset_pdf_start;

        return ERR_NONE;
    }

    if (sgl->pdf_data.file != NULL) {

        file_position = ftell(sgl->pdf_data.file);
        if (file_position < 0 || sgl->offset_pdf_start > file_position)
            return ERR_IO;

        *result = file_position - sgl->offset_pdf_start;

        return ERR_NONE;
    }

    return ERR_NO_DATA;
}

sigil_err_t skip_leading_whitespaces(sigil_t *sgl)
{
    sigil_err_t err;
    char c;

    while((err = pdf_peek_char(sgl, &c)) == ERR_NONE) {
        if (!is_whitespace(c))
            return ERR_NONE;

        err = pdf_move_pos_rel(sgl, 1);
        if (err != ERR_NONE)
            return err;
    }

    return err;
}

sigil_err_t skip_array(sigil_t *sgl)
{
    sigil_err_t err;
    char c;

    while((err = pdf_get_char(sgl, &c)) == ERR_NONE) {
        switch (c) {
            case '[':
                err = skip_array(sgl);
                if (err != ERR_NONE)
                    return err;
                break;
            case ']':
                return ERR_NONE;
            case EOF:
                return ERR_PDF_CONTENT;
            default:
                break;
        }
    }

    return err;
}

sigil_err_t skip_dictionary(sigil_t *sgl)
{
    sigil_err_t err;
    char c;

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NONE)
        return err;

    while ((err = pdf_get_char(sgl, &c)) == ERR_NONE) {
        switch (c) {
            case '>':
                if ((err = pdf_get_char(sgl, &c)) != ERR_NONE)
                    return err;
                if (c == '>')
                    return ERR_NONE;
                break;
            case '<':
                if ((err = pdf_get_char(sgl, &c)) != ERR_NONE)
                    return err;
                if (c != '<')
                    break;
                if ((err = skip_dictionary(sgl)) != ERR_NONE)
                    return err;
                break;
            case EOF:
                return ERR_PDF_CONTENT;
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
    int first_non_whitespace = 1;

    while ((err = pdf_peek_char(sgl, &c)) == ERR_NONE) {
        if (first_non_whitespace && !is_whitespace(c)) {
            first_non_whitespace = 0;

            switch (c) {
                case '/':
                    if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
                        return err;
                    break;
                case '[':
                    if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
                        return err;
                    if ((err = skip_array(sgl)) != ERR_NONE)
                        return err;
                    return ERR_NONE;
                case '<':
                    if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
                        return err;
                    if ((err = pdf_peek_char(sgl, &c)) != ERR_NONE)
                        return err;
                    if (c != '<')
                        break;
                    if ((err = skip_dictionary(sgl)) != ERR_NONE)
                        return err;
                    return ERR_NONE;
                default:
                    break;
            }

            continue;
        }

        switch (c) {
            case '/': // key of next entry
            case '>': // end of dictionary
                return ERR_NONE;
            case EOF:
                return ERR_PDF_CONTENT;
            default:
                if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
                    return err;
                break;
        }
    }

    return err;
}

sigil_err_t skip_word(sigil_t *sgl, const char *word)
{
    sigil_err_t err;
    size_t length;
    char c;

    if (sgl == NULL || word == NULL)
        return ERR_PARAMETER;

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NONE)
        return err;

    length = strlen(word);

    for (size_t pos = 0; pos < length; pos++) {
        err = pdf_peek_char(sgl, &c);
        if (err != ERR_NONE)
            return err;

        if (c != word[pos])
            return ERR_PDF_CONTENT;

        err = pdf_move_pos_rel(sgl, 1);
        if (err != ERR_NONE)
            return err;
    }

    return ERR_NONE;
}

sigil_err_t parse_number(sigil_t *sgl, size_t *number)
{
    sigil_err_t err;
    char c;
    int digits = 0;

    if (sgl == NULL || number == NULL)
        return ERR_PARAMETER;

    *number = 0;

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NONE)
        return err;

    // number
    while ((err = pdf_peek_char(sgl, &c)) == ERR_NONE) {
        if (!is_digit(c)) {
            if (digits > 0) {
                return ERR_NONE;
            } else {
                return ERR_PDF_CONTENT;
            }
        }

        *number = 10 * (*number) + c - '0';

        if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
            return err;

        digits++;
    }

    return err;
}

sigil_err_t parse_indirect_reference(sigil_t *sgl, reference_t *ref)
{
    sigil_err_t err;

    err = parse_number(sgl, &ref->object_num);
    if (err != ERR_NONE)
        return err;

    err = parse_number(sgl, &ref->generation_num);
    if (err != ERR_NONE)
        return err;

    err = skip_word(sgl, "R");
    if (err != ERR_NONE)
        return err;

    return ERR_NONE;
}

// parse the key of the pair key - value in the dictionary
sigil_err_t parse_dict_key(sigil_t *sgl, dict_key_t *dict_key)
{
    sigil_err_t err;
    int count = 0;
    char tmp[DICT_KEY_MAX],
         c;

    if (sgl == NULL || dict_key == NULL)
        return ERR_PARAMETER;

    sigil_zeroize(tmp, DICT_KEY_MAX * sizeof(*tmp));

    if (skip_word(sgl, ">>") == ERR_NONE)
        return ERR_END_OF_DICT;

    err = skip_word(sgl, "/");
    if (err != ERR_NONE)
        return err;

    while ((err = pdf_peek_char(sgl, &c)) == ERR_NONE) {
        if (is_whitespace(c)) {
            if (count <= 0)
                return ERR_PDF_CONTENT;
            break;
        } else {
            if (count >= DICT_KEY_MAX - 1)
                return ERR_PDF_CONTENT;
            tmp[count++] = c;
        }

        if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
            return err;
    }

    tmp[count] = '\0';

    if (err != ERR_NONE)
        return err;

    if (strncmp(tmp, "Size", 4) == 0) {
        *dict_key = DICT_KEY_Size;
    } else if (strncmp(tmp, "Prev", 4) == 0) {
        *dict_key = DICT_KEY_Prev;
    } else if (strncmp(tmp, "Root", 4) == 0) {
        *dict_key = DICT_KEY_Root;
    } else if (strncmp(tmp, "AcroForm", 8) == 0) {
        *dict_key = DICT_KEY_AcroForm;
    } else if (strncmp(tmp, "Fields", 6) == 0) {
        *dict_key = DICT_KEY_Fields;
    } else if (strncmp(tmp, "SigFlags", 8) == 0) {
        *dict_key = DICT_KEY_SigFlags;
    } else if (strncmp(tmp, "FT", 2) == 0) {
        *dict_key = DICT_KEY_FT;
    } else if (strncmp(tmp, "V", 1) == 0) {
        *dict_key = DICT_KEY_V;
    } else if (strncmp(tmp, "SubFilter", 9) == 0) {
        *dict_key = DICT_KEY_SubFilter;
    } else if (strncmp(tmp, "Cert", 4) == 0) {
        *dict_key = DICT_KEY_Cert;
    } else if (strncmp(tmp, "Contents", 8) == 0) {
        *dict_key = DICT_KEY_Contents;
    } else if (strncmp(tmp, "ByteRange", 9) == 0) {
        *dict_key = DICT_KEY_ByteRange;
    } else {
        *dict_key = DICT_KEY_UNKNOWN;
    }

    sigil_zeroize(tmp, DICT_KEY_MAX * sizeof(*tmp));

    return ERR_NONE;
}

// parsing array of indirect references into ref_array
sigil_err_t parse_ref_array(sigil_t *sgl, ref_array_t *ref_array)
{
    sigil_err_t err;
    size_t position;
    reference_t reference;

    if (sgl == NULL || ref_array == NULL)
        return ERR_PARAMETER;

    if ((err = skip_word(sgl, "[")) != ERR_NONE)
        return err;

    if (skip_word(sgl, "]") == ERR_NONE) // empty array
        return ERR_NONE;

    if (ref_array->capacity <= 0) {
        ref_array->entry = malloc(sizeof(*ref_array->entry) * REF_ARRAY_PREALLOCATION);
        if (ref_array->entry == NULL)
            return ERR_ALLOCATION;
        sigil_zeroize(ref_array->entry, sizeof(*ref_array->entry) * REF_ARRAY_PREALLOCATION);
        ref_array->capacity = REF_ARRAY_PREALLOCATION;
    }

    position = 0;

    while ((err = parse_indirect_reference(sgl,&reference)) == ERR_NONE) {
        if (position >= ref_array->capacity) {
            ref_array->entry = realloc(ref_array->entry,
                sizeof(*ref_array->entry) * ref_array->capacity * 2);

            if (ref_array->entry == NULL)
                return ERR_ALLOCATION;

            sigil_zeroize(ref_array->entry + ref_array->capacity,
                sizeof(*ref_array->entry) * ref_array->capacity);

            ref_array->capacity *= 2;
        }

        if (ref_array->entry[position] == NULL) {
            ref_array->entry[position] = malloc(sizeof(reference_t));
            if (ref_array->entry[position] == NULL)
                return ERR_ALLOCATION;
        }

        ref_array->entry[position]->object_num = reference.object_num;
        ref_array->entry[position]->generation_num = reference.generation_num;

        if (skip_word(sgl, "]") == ERR_NONE)
            return ERR_NONE;

        position++;
    }

    return err;
}

sigil_err_t reference_to_offset(sigil_t *sgl, const reference_t *ref, size_t *result)
{
    xref_entry_t *xref_entry;

    if (sgl == NULL || ref == NULL || sgl->xref == NULL || result == NULL)
        return ERR_PARAMETER;

    if (sgl->xref->capacity <= ref->object_num)
        return ERR_NO_DATA;

    xref_entry = sgl->xref->entry[ref->object_num];

    while (xref_entry != NULL) {
        if (xref_entry->generation_num == ref->generation_num) {
            *result = xref_entry->byte_offset;
            return ERR_NONE;
        }

        xref_entry = xref_entry->next;
    }

    return ERR_NO_DATA;
}

void print_module_name(const char *module_name, int verbosity)
{
    if (verbosity < 1 || module_name == NULL)
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
    if (verbosity < 2 || test_name == NULL)
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

sigil_t *test_prepare_sgl_buffer(char *content, size_t size)
{
    sigil_t *sgl;

    if (content == NULL)
        return NULL;

    if (sigil_init(&sgl) != ERR_NONE)
        return NULL;

    if (sigil_set_pdf_buffer(sgl, content, size) != ERR_NONE) {
        sigil_free(&sgl);
        return NULL;
    }

    return sgl;
}

sigil_t *test_prepare_sgl_path(const char *path)
{
    sigil_t *sgl;

    if (path == NULL)
        return NULL;

    if (sigil_init(&sgl) != ERR_NONE)
        return NULL;

    if (sigil_set_pdf_path(sgl, path) != ERR_NONE) {
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

    // TEST: fn pdf_read
    print_test_item("fn pdf_read", verbosity);

    {
        char output[6];
        size_t output_size;

        char *sstream = "abbbcx";
        if ((sgl = test_prepare_sgl_buffer(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (pdf_read(sgl, 5, output, &output_size) != ERR_NONE ||
            output_size != 5 || strncmp(sstream, output, 5) != 0)
        {
            goto failed;
        }

        if ((pdf_get_char(sgl, &c)) != ERR_NONE || c != 'x')
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn skip_leading_whitespaces
    print_test_item("fn skip_leading_whitespaces", verbosity);

    {
        if ((sgl = test_prepare_sgl_buffer("x", 2)) == NULL)
            goto failed;

        if (skip_leading_whitespaces(sgl) != ERR_NONE)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NONE || c != 'x')
            goto failed;

        sigil_free(&sgl);

        if ((sgl = test_prepare_sgl_buffer("\x00\x09\x0a\x0c\x0d\x20x", 8)) == NULL)
            goto failed;

        if (skip_leading_whitespaces(sgl) != ERR_NONE)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NONE || c != 'x')
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
        if ((sgl = test_prepare_sgl_buffer(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (skip_dictionary(sgl) != ERR_NONE)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NONE || c != 'x')
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
        if ((sgl = test_prepare_sgl_buffer(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (skip_dict_unknown_value(sgl) != ERR_NONE)
            goto failed;

        if ((pdf_get_char(sgl, &c)) != ERR_NONE || c != 'x')
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn parse_number
    print_test_item("fn parse_number", verbosity);

    {
        size_t result = 0;

        char *sstream = "0123456789    42";
        if ((sgl = test_prepare_sgl_buffer(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (parse_number(sgl, &result) != ERR_NONE || result != 123456789)
            goto failed;

        if (parse_number(sgl, &result) != ERR_NONE || result != 42)
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: UTF-8 filepath support
    print_test_item("UTF-8 filepath support", verbosity);

    {
        if ((sgl = test_prepare_sgl_path("test/utf-8_test_\xe2\x82\xac\xe4\xaa"\
            "\x9c\xe5\x8b\x81\xf0\xa0\xb9\xb9")) == NULL)
        {
            goto failed;
        }

        if ((pdf_get_char(sgl, &c)) != ERR_NONE || c != 'x')
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
