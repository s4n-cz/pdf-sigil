/** @file
 *
 */

#ifndef PDF_SIGIL_SIG_FIELD_H
#define PDF_SIGIL_SIG_FIELD_H

#include "types.h"

/** @brief Go through all the objects mentioned in the Fields entry from the
 *         interactive form dictionary (AcroForm) and look for the signature
 *         field. If found, save the object reference inside the context
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t find_sig_field(sigil_t *sgl);

/** @brief Does processing of the signature field and saves the position of the
 *         signature dictionary (V entry). Doesn't depend on the current
 *         position in the PDF
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t process_sig_field(sigil_t *sgl);

/** @brief Tests for the sig_field module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_sig_field_self_test(int verbosity);

#endif /* PDF_SIGIL_SIG_FIELD_H */
