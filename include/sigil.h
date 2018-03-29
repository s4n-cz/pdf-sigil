#ifndef PDF_SIGIL_SIGIL_H
#define PDF_SIGIL_SIGIL_H

#include "types.h"


sigil_err_t sigil_init(sigil_t **sgl);

sigil_err_t sigil_set_pdf_file(sigil_t *sgl, FILE *pdf_file);
sigil_err_t sigil_set_pdf_path(sigil_t *sgl, const char *path_to_pdf);
sigil_err_t sigil_set_pdf_buffer(sigil_t *sgl, char *pdf_content, size_t size);

sigil_err_t sigil_set_trusted_default_system(sigil_t *sgl);
sigil_err_t sigil_set_trusted_file(sigil_t *sgl, const char *path_to_file);
sigil_err_t sigil_set_trusted_dir(sigil_t *sgl, const char *path_to_dir);

sigil_err_t sigil_verify(sigil_t *sgl);

sigil_err_t sigil_get_result(sigil_t *sgl, int *result);
sigil_err_t sigil_get_cert_validation_result(sigil_t *sgl, int *result);
sigil_err_t sigil_get_data_integrity_result(sigil_t *sgl, int *result);
sigil_err_t sigil_get_original_digest(sigil_t *sgl, ASN1_OCTET_STRING **digest);
sigil_err_t sigil_get_computed_digest(sigil_t *sgl, ASN1_OCTET_STRING **digest);
sigil_err_t sigil_get_subfilter(sigil_t *sgl, int *subfilter);

void cert_free(cert_t *cert);

void contents_free(sigil_t *sgl);

void sigil_free(sigil_t **sgl);

int sigil_sigil_self_test(int verbosity);

#endif /* PDF_SIGIL_SIGIL_H */
