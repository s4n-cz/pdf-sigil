#ifndef PDF_SIGIL_AUXILIARY_H
#define PDF_SIGIL_AUXILIARY_H

#include <stdio.h>
#include <stdint.h>

#ifndef CHAR_T
#define CHAR_T
    typedef unsigned char char_t;
#endif /* CHAR_T */

#ifndef SIGIL_ERR_T
#define SIGIL_ERR_T
    typedef uint32_t sigil_err_t;
#endif /* SIGIL_ERR_T */

#ifndef KEYWORD_T
#define KEYWORD_T
    typedef uint32_t keyword_t;
#endif /* KEYWORD_T */

#ifndef FREE_INDICATOR_T
#define FREE_INDICATOR_T
    typedef uint32_t free_indicator_t;
#endif /* FREE_INDICATOR_T */

#ifndef REREFENCE_T
#define REREFENCE_T
    typedef struct {
        size_t object_num;
        size_t generation_num;
    } reference_t;
#endif /* REREFENCE_T */

#ifndef DICT_KEY_T
#define DICT_KEY_T
    typedef uint32_t dict_key_t;
#endif /* DICT_KEY_T */

#define KEYWORD_xref     0
#define KEYWORD_trailer  1

#define IN_USE_ENTRY     0
#define FREE_ENTRY       1

#define DICT_KEY_Size      0
#define DICT_KEY_Prev      1
#define DICT_KEY_Root      2
#define DICT_KEY_unknown   3

#define COLOR_RED        "\x1b[31m"
#define COLOR_GREEN      "\x1b[32m"
#define COLOR_RESET      "\x1b[0m"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Shouldn't be optimized out by the compiler
void sigil_zeroize(void *a, size_t bytes);

int is_digit(const char_t c);
int is_whitespace(const char_t c);

sigil_err_t skip_leading_whitespaces(FILE *in);
sigil_err_t skip_dictionary(FILE *in);
sigil_err_t skip_dict_unknown_value(FILE *in);

sigil_err_t parse_number(FILE *in, size_t *number);
sigil_err_t parse_keyword(FILE *in, keyword_t *keyword);
sigil_err_t parse_free_indicator(FILE *in, free_indicator_t *result);
sigil_err_t parse_indirect_reference(FILE *in, reference_t *ref);
sigil_err_t parse_dict_key(FILE *in, dict_key_t *dict_key);

void print_module_name(const char *module_name, int verbosity);
void print_module_result(int result, int verbosity);
void print_test_item(const char *test_name, int verbosity);
void print_test_result(int result, int verbosity);

int sigil_auxiliary_self_test(int verbosity);

#endif /* PDF_SIGIL_AUXILIARY_H */
