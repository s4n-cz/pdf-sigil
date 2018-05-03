/** @file
 *
 */

#ifndef PDF_SIGIL_SIG_DICT_H
#define PDF_SIGIL_SIG_DICT_H

#include "types.h"

/** @brief Does the signature dictionary processing. Does't depend on the current
 *         position in the PDF
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t process_sig_dict(sigil_t *sgl);

/** @brief Tests for the sig_dict module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_sig_dict_self_test(int verbosity);

#endif /* PDF_SIGIL_SIG_DICT_H */
