/** @file
 *
 */

#ifndef PDF_SIGIL_SIGIL_H
#define PDF_SIGIL_SIGIL_H

#include "types.h"

/** @brief Does initialization of the provided context. Allocates the structure
 *         and sets default values
 *
 * @param sgl pointer to the sigil_t context to be initialized.
 * @return ERR_NONE if success
 */
sigil_err_t sigil_init(sigil_t **sgl);

/** @brief Sets the provided file to the context. If the size is smaller than
 *         the THRESHOLD_FILE_BUFFERING, allocates a new buffer and makes a copy
 *         of the PDF data
 *
 * @param sgl context
 * @param pdf_file input - file pointer with the PDF data
 * @return ERR_NONE if success
 */
sigil_err_t sigil_set_pdf_file(sigil_t *sgl, FILE *pdf_file);

/** @brief Opens a file from the provided filepath and calls sigil_set_pdf_file
 *
 * @param sgl context
 * @param path_to_pdf input - path to the PDF file
 * @return ERR_NONE if success
 */
sigil_err_t sigil_set_pdf_path(sigil_t *sgl, const char *path_to_pdf);

/** @brief Sets the provided buffer with the PDF data to the context
 *
 * @param sgl context
 * @param pdf_content input - buffer containing the PDF data
 * @param size size of the pdf_content buffer
 * @return ERR_NONE if success
 */
sigil_err_t sigil_set_pdf_buffer(sigil_t *sgl, char *pdf_content, size_t size);

/** @brief Sets the default system storage of the trusted CA certificates to the
 *         context for later certificate verification
 *
 * @param sgl context
 * @return ERR_NONE if success
 */
sigil_err_t sigil_set_trusted_system(sigil_t *sgl);

/** @brief Loads the provided certificate to the storage of trusted certificates
 *
 * @param sgl context
 * @param path_to_file input - path to the file containing a certificate
 * @return ERR_NONE if success
 */
sigil_err_t sigil_set_trusted_file(sigil_t *sgl, const char *path_to_file);

/** @brief Loads all the certificates contained inside of the provided folder to
 *         the storage of trusted certificates. The certificates inside of the
 *         folder must be in a hashed form as documented in OpenSSL function
 *         X509_LOOKUP_hash_dir.
 *
 * @param sgl context
 * @param path_to_dir input - path to the directory containing a certificates in
 *                    a hashed form
 * @return ERR_NONE if success
 */
sigil_err_t sigil_set_trusted_dir(sigil_t *sgl, const char *path_to_dir);

/** @brief Verifies the digital signature and saves the result in the context.
 *         In order to get the result, call sigil_get_result
 *
 * @param sgl context
 * @return ERR_NONE if success (NOT the result of actual verification)
 */
sigil_err_t sigil_verify(sigil_t *sgl);

/** @brief Gets the result from the provided context
 *
 * @param sgl context
 * @param result output - the result of digital signature verification
 * @return ERR_NONE if success
 */
sigil_err_t sigil_get_result(sigil_t *sgl, int *result);

/** @brief Gets the result of a certificate validation phase from the provided
 *         context
 *
 * @param sgl context
 * @param result output - result of the certificate validation
 * @return ERR_NONE if success
 */
sigil_err_t sigil_get_cert_validation_result(sigil_t *sgl, int *result);

/** @brief Gets the result of a data integrity (message digest comparison) phase
 *         from the provided context
 *
 * @param sgl context
 * @param result output - result of the message digest comparison
 * @return ERR_NONE if success
 */
sigil_err_t sigil_get_data_integrity_result(sigil_t *sgl, int *result);

/** @brief Gets the original message digest (from the signature) from the
 *         provided context
 *
 * @param sgl context
 * @param digest output - the original message digest (from the signature)
 * @return ERR_NONE if success
 */
sigil_err_t sigil_get_original_digest(sigil_t *sgl, ASN1_OCTET_STRING **digest);

/** @brief Gets the computed message digest from the provided context
 *
 * @param sgl context
 * @param digest output - the computed message digest
 * @return ERR_NONE if success
 */
sigil_err_t sigil_get_computed_digest(sigil_t *sgl, ASN1_OCTET_STRING **digest);

/** @brief Gets the subfilter value from the provided context
 *
 * @param sgl context
 * @param subfilter output - the subfiter value
 * @return ERR_NONE if success
 */
sigil_err_t sigil_get_subfilter(sigil_t *sgl, int *subfilter);

/** @brief Cleans-up the provided sigil context
 *
 * @param sgl context
 */
void sigil_free(sigil_t **sgl);

/** @brief Returns message with an error explanation to the provided error code
 *
 * @param err input - the error code
 * @return the error message
 */
const char *sigil_err_string(sigil_err_t err);

/** @brief Tests for sigil module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_sigil_self_test(int verbosity);

#endif /* PDF_SIGIL_SIGIL_H */
