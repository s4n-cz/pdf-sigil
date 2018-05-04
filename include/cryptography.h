/** @file
 *
 */

#ifndef PDF_SIGIL_CRYPTOGRAPHY_H
#define PDF_SIGIL_CRYPTOGRAPHY_H

#include "types.h"


void print_computed_hash(sigil_t *sgl);

/** @brief Compute a message digest (hash) for the PKCS#1 signature type
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t compute_digest_pkcs1(sigil_t *sgl);

/** @brief Load certificates from the hex form to the X.509 object
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t load_certificates(sigil_t *sgl);

/** @brief Get the original message digest from the loaded hexadecimal form of the
 *         Contents entry from the signature dictionary
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t load_digest(sigil_t *sgl);

/** @brief Verify validity of the signing certificate. If present, it is using
 *         the other provided certificates to build the chain of trust. Does
 *         save the result inside of the context (NOT the return value)
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t verify_signing_certificate(sigil_t *sgl);

/** @brief Compare the message digest from the signature with the computed one.
 *         Does save the result inside of the context (NOT the return value)
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t compare_digest(sigil_t *sgl);

/** @brief Tests for the cryptography module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_cryptography_self_test(int verbosity);

#endif /* PDF_SIGIL_CRYPTOGRAPHY_H */
