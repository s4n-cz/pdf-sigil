/** @file
 *
 */

#ifndef PDF_SIGIL_XREF_H
#define PDF_SIGIL_XREF_H

#include "types.h"

/** @brief Allocates a new xref structure and sets default values
 *
 * @return valid xref_t structure or NULL if error occured
 */
xref_t *xref_init(void);

/** @brief Clean-up of the provided xref structure
 *
 * @param xref the structure to be freed
 */
void xref_free(xref_t *xref);

/** @brief Read the offset of the last cross-reference section
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t read_startxref(sigil_t *sgl);

/** @brief Reads all the entries from the cross-reference table to the context
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t read_xref_table(sigil_t *sgl);

/** @brief Does processing of the cross-reference section - determines type and
 *         reads the entries
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t process_xref(sigil_t *sgl);

/** @brief For debugging purposes - print all the data from the provided
 *         cross-reference table
 *
 * @param xref input cross-reference table to be printed
 */
void print_xref(xref_t *xref);

/** @brief Tests for the xref module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_xref_self_test(int verbosity);

#endif /* PDF_SIGIL_XREF_H */
