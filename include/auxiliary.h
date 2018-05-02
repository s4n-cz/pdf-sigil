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

/** @brief Decides whether the character is a digit
 *
 * @param c character provided for comparison
 * @return 1 if true, 0 otherwise
 */
int is_digit(const char c);

/** @brief Decides whether the character is a whitespace according to PDF standard
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

/** @brief Move position in the PDF forward to the first non-whitespace character
 *         from the current position
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t skip_leading_whitespaces(sigil_t *sgl);

/** @brief Move position in the PDF after the array from the current position.
 *         The initial position needs to be after the leading '[' character
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t skip_array(sigil_t *sgl);

/** @brief Move position in the PDF after the dictionary from the current position.
 *         The initial position needs to be after the leading "<<" characters
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t skip_dictionary(sigil_t *sgl);

/** @brief Move position in the PDF after the value (pairs of key-value) inside
 *         of the dictionary from the current position
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t skip_dict_unknown_value(sigil_t *sgl);

/** @brief Move position in the PDF after the specified word that needs to start
 *         from the current possition (leading whitespaces are allowed)
 *
 * @param sgl context
 * @param word move position after this word
 * @return ERR_NONE if success
 */
sigil_err_t skip_word(sigil_t *sgl, const char *word);

/** @brief Loads a number from the current position in the PDF
 *
 * @param sgl context
 * @param number output - loaded number
 * @return ERR_NONE if success
 */
sigil_err_t parse_number(sigil_t *sgl, size_t *number);

/** @brief Loads an indirect reference from the current position in the PDF
 *
 * @param sgl context
 * @param ref output - indirect reference loaded
 * @return ERR_NONE if success
 */
sigil_err_t parse_indirect_reference(sigil_t *sgl, reference_t *ref);

/** @brief Loads a dictionary key from the current position in the PDF
 *
 * @param sgl context
 * @param dict_key output - loaded key
 * @return ERR_NONE if success
 */
sigil_err_t parse_dict_key(sigil_t *sgl, dict_key_t *dict_key);

/** @brief Loads an array of indirect references from the current position in
 *         the PDF. The leading '[' needs to be included
 *
 * @param sgl context
 * @param ref_array output - the loaded array of indirect references
 * @return ERR_NONE if success
 */
sigil_err_t parse_ref_array(sigil_t *sgl, ref_array_t *ref_array);

/** @brief Resolves the offset of an object according to the xref section
 *
 * @param sgl context
 * @param ref input - indirect reference to be resolved
 * @param result output - the byte offset
 * @return ERR_NONE if success
 */
sigil_err_t reference_to_offset(sigil_t *sgl, const reference_t *ref, size_t *result);

/** @brief Print the module name, according to the verbosity level set
 *
 * @param module_name name of the module to be printed
 * @param verbosity with values >= 1 the output is printed
 */
void print_module_name(const char *module_name, int verbosity);

/** @brief Print nicely the overall results of the tested module
 *
 * @param result value 1 means the module passed all the tests, 0 means it failed
 * @param verbosity with values >= 1 the output is printed
 */
void print_module_result(int result, int verbosity);

/** @brief Print the name of a test inside of a module
 *
 * @param test_name the name to be printed
 * @param verbosity with values >= 2 the output is printed
 */
void print_test_item(const char *test_name, int verbosity);

/** @brief Print the result of one test inside of a module
 *
 * @param result value 1 means the test succeeded, 0 means it failed
 * @param verbosity with values >= 2 the output is printed
 */
void print_test_result(int result, int verbosity);

/** @brief Prepares context for the tests from the provided buffer and size
 *
 * @param content the PDF data
 * @param size number of bytes of the content
 * @return valid sigil_t context if succeeded, NULL if failed
 */
sigil_t *test_prepare_sgl_buffer(char *content, size_t size);

/** @brief Prepares context for the tests from a file specified by filepath
 *
 * @param path the PDF filepath
 * @return valid sigil_t context if succeeded, NULL if failed
 */
sigil_t *test_prepare_sgl_path(const char *path);

/** @brief Tests for the auxiliary module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_auxiliary_self_test(int verbosity);

#endif /* PDF_SIGIL_AUXILIARY_H */
