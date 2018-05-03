/** @file
 *
 */

#ifndef PDF_SIGIL_TRAILER_H
#define PDF_SIGIL_TRAILER_H

#include "types.h"

/** @brief Process the trailer section. Initial position in the PDF needs to be
 *         at the beginning of that section
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t process_trailer(sigil_t *sgl);

/** @brief Tests for trailer module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_trailer_self_test(int verbosity);

#endif /* PDF_SIGIL_TRAILER_H */
