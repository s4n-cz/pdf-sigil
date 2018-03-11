#ifndef PDF_SIGIL_AUXILIARY_H
#define PDF_SIGIL_AUXILIARY_H

#include <stdio.h> // size_t, FILE
#include "types.h"


#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Shouldn't be optimized out by the compiler
void sigil_zeroize(void *a, size_t bytes);

int is_digit(const char c);
int is_whitespace(const char c);

sigil_err_t pdf_read(sigil_t *sgl, size_t size, char *result, size_t *res_size);
sigil_err_t pdf_get_char(sigil_t *sgl, char *result);
sigil_err_t pdf_peek_char(sigil_t *sgl, char *result);

sigil_err_t pdf_move_pos_rel(sigil_t *sgl, ssize_t shift_bytes);
sigil_err_t pdf_move_pos_abs(sigil_t *sgl, size_t position);
sigil_err_t pdf_goto_obj(sigil_t *sgl, reference_t *ref);

sigil_err_t get_curr_position(sigil_t *sgl, size_t *result);

sigil_err_t skip_leading_whitespaces(sigil_t *sgl);
sigil_err_t skip_array(sigil_t *sgl);
sigil_err_t skip_dictionary(sigil_t *sgl);
sigil_err_t skip_dict_unknown_value(sigil_t *sgl);

sigil_err_t parse_number(sigil_t *sgl, size_t *number);
sigil_err_t parse_word(sigil_t *sgl, const char *word);
sigil_err_t parse_indirect_reference(sigil_t *sgl, reference_t *ref);
sigil_err_t parse_dict_key(sigil_t *sgl, dict_key_t *dict_key);
sigil_err_t parse_ref_array(sigil_t *sgl, ref_array_t *ref_array);

sigil_err_t reference_to_offset(sigil_t *sgl, const reference_t *ref, size_t *result);

const char *sigil_err_string(sigil_err_t err);

void print_module_name(const char *module_name, int verbosity);
void print_module_result(int result, int verbosity);
void print_test_item(const char *test_name, int verbosity);
void print_test_result(int result, int verbosity);

sigil_t *test_prepare_sgl_content(char *content, size_t size);
sigil_t *test_prepare_sgl_path(const char *path);

int sigil_auxiliary_self_test(int verbosity);

#endif /* PDF_SIGIL_AUXILIARY_H */
