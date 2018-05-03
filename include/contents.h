/** @file
 *
 */

#ifndef PDF_SIGIL_CONTENTS_H
#define PDF_SIGIL_CONTENTS_H

#include "types.h"

/** @brief Load the data from Contents entry in the signature dictionary
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t parse_contents(sigil_t *sgl);

/** @brief Tests for the contents module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_contents_self_test(int verbosity);

#endif //PDF_SIGIL_CONTENTS_H
