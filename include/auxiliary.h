/** @file
 *
 */

#ifndef PDF_SIGIL_AUXILIARY_H
#define PDF_SIGIL_AUXILIARY_H

#include <stdio.h> // size_t, FILE
#include "types.h"

/** @brief Find minimum of two values
 *
 */
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
/** @brief Find maximum of two values
 *
 */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/** @brief Fills content with zeros, shouldn't be optimized out by the compiler
 *
 * @param a starting address
 * @param bytes number of bytes
 */
void sigil_zeroize(void *a, size_t bytes);

/** @brief Decide whether the character is a digit
 *
 * @param c character provided for comparison
 * @return 1 if true, 0 otherwise
 */
int is_digit(const char c);

/** @brief Decide whether the character is a whitespace according to PDF standard
 *
 * @param c character provided for comparison
 * @return 1 if true, 0 otherwise
 */
int is_whitespace(const char c);

/** @brief Reads *size* bytes from PDF to *result* and adds a terminating null.
 *         Does move the position in PDF.
 *
 * @param sgl context
 * @param size number of bytes
 * @param result output buffer
 * @param res_size output number of bytes read, doesn't include terminating null
 * @return ERR_NONE if success
 */
sigil_err_t pdf_read(sigil_t *sgl, size_t size, char *result, size_t *res_size);

/** @brief Reads one character from PDF and moves the position in PDF.
 *
 * @param sgl context
 * @param result output character
 * @return ERR_NONE if success
 */
sigil_err_t pdf_get_char(sigil_t *sgl, char *result);

/** @brief Reads one character from PDF and **does not** move the position in PDF
 *
 * @param sgl context
 * @param result output character
 * @return ERR_NONE if success
 */
sigil_err_t pdf_peek_char(sigil_t *sgl, char *result);

/** @brief Moves position in the PDF relatively to the current position
 *
 * @param sgl context
 * @param shift_bytes number of bytes (positive or negative)
 * @return ERR_NONE if success
 */
sigil_err_t pdf_move_pos_rel(sigil_t *sgl, ssize_t shift_bytes);

/** @brief Moves position in the PDF to the specified value
 *
 * @param sgl context
 * @param position final position in the PDF
 * @return ERR_NONE if success
 */
sigil_err_t pdf_move_pos_abs(sigil_t *sgl, size_t position);

/** @brief Moves position to the object specified as an indirect reference.
 *         Skips leading object identifiers (X Y obj)
 *
 * @param sgl context
 * @param ref object indirect reference
 * @return ERR_NONE if success
 */
sigil_err_t pdf_goto_obj(sigil_t *sgl, reference_t *ref);

/** @brief Gets the current position inside the PDF
 *
 * @param sgl context
 * @param result output - the current position in the PDF
 * @return ERR_NONE if success
 */
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

void print_module_name(const char *module_name, int verbosity);
void print_module_result(int result, int verbosity);
void print_test_item(const char *test_name, int verbosity);
void print_test_result(int result, int verbosity);

sigil_t *test_prepare_sgl_content(char *content, size_t size);
sigil_t *test_prepare_sgl_path(const char *path);

int sigil_auxiliary_self_test(int verbosity);

#endif /* PDF_SIGIL_AUXILIARY_H */
