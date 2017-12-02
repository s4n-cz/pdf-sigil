#ifndef PDF_SIGIL_XREF_H
#define PDF_SIGIL_XREF_H

#include "error.h"
#include "sigil.h"


sigil_err_t read_startxref(sigil_t *sgl);

int sigil_xref_self_test(int verbosity);

#endif /* PDF_SIGIL_XREF_H */
