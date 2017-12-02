#ifndef PDF_SIGIL_HEADER_H
#define PDF_SIGIL_HEADER_H

#include "error.h"
#include "sigil.h"


sigil_err_t process_header(sigil_t *sgl);

int sigil_header_self_test(int verbosity);

#endif /* PDF_SIGIL_HEADER_H */
