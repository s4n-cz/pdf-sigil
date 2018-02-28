#ifndef PDF_SIGIL_SIGIL_H
#define PDF_SIGIL_SIGIL_H

#include "types.h"


sigil_err_t sigil_init(sigil_t **sgl);

sigil_err_t sigil_verify(sigil_t *sgl, const char *filepath);

// ... get functions TODO

void sigil_free(sigil_t *sgl);

int sigil_sigil_self_test(int verbosity);

#endif /* PDF_SIGIL_SIGIL_H */
