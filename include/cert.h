/** @file
 *
 */

#ifndef PDF_SIGIL_CERT_H
#define PDF_SIGIL_CERT_H

#include "types.h"

/** @brief Read the certificates in hex, the first one is the signing certificate
 *         and the following are for verifying the authenticity of the signing one
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t parse_certs(sigil_t *sgl);

/** @brief Tests for the cert module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_cert_self_test(int verbosity);

#endif /* PDF_SIGIL_CERT_H */
