#ifndef PDF_SIGIL_AUXILIARY_H
#define PDF_SIGIL_AUXILIARY_H

#include <stdio.h> // size_t, FILE
#include "types.h"


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

const char *sigil_err_string(sigil_err_t err);

void print_module_name(const char *module_name, int verbosity);
void print_module_result(int result, int verbosity);
void print_test_item(const char *test_name, int verbosity);
void print_test_result(int result, int verbosity);

int sigil_auxiliary_self_test(int verbosity);

#endif /* PDF_SIGIL_AUXILIARY_H */
