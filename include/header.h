/** @file
 *
 */

#ifndef PDF_SIGIL_HEADER_H
#define PDF_SIGIL_HEADER_H

#include "types.h"

/** @brief Read the header at the beginning of the PDF and get the PDF version
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t process_header(sigil_t *sgl);

/** @brief Tests for the header module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_header_self_test(int verbosity);

#endif /* PDF_SIGIL_HEADER_H */
