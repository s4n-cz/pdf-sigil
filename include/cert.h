/** @file
 *
 */

#ifndef PDF_SIGIL_CERT_H
#define PDF_SIGIL_CERT_H

#include "types.h"


sigil_err_t parse_certs(sigil_t *sgl);

int sigil_cert_self_test(int verbosity);

#endif /* PDF_SIGIL_CERT_H */
