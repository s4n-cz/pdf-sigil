#ifndef PDF_SIGIL_CRYPTOGRAPHY_H
#define PDF_SIGIL_CRYPTOGRAPHY_H

#include "types.h"


void print_computed_hash(sigil_t *sgl);

sigil_err_t compute_sha1_hash_over_range(sigil_t *sgl);

sigil_err_t load_certificates(sigil_t *sgl);

sigil_err_t verify_signing_certificate(sigil_t *sgl);

int sigil_sigil_self_test(int verbosity);

#endif /* PDF_SIGIL_CRYPTOGRAPHY_H */
