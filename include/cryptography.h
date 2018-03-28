#ifndef PDF_SIGIL_CRYPTOGRAPHY_H
#define PDF_SIGIL_CRYPTOGRAPHY_H

#include "types.h"


sigil_err_t hex_to_dec(const char *in, size_t in_len, unsigned char *out, size_t *out_len);

void print_computed_hash(sigil_t *sgl);

sigil_err_t compute_digest_pkcs1(sigil_t *sgl);

sigil_err_t load_certificates(sigil_t *sgl);

sigil_err_t load_digest(sigil_t *sgl);

sigil_err_t verify_signing_certificate(sigil_t *sgl);

sigil_err_t compare_digest(sigil_t *sgl);

sigil_err_t verify_digest(sigil_t *sgl, int *result);

int sigil_cryptography_self_test(int verbosity);

#endif /* PDF_SIGIL_CRYPTOGRAPHY_H */
