#ifndef PDF_SIGIL_XREF_H
#define PDF_SIGIL_XREF_H

#include "types.h"


xref_t *xref_init();

void xref_free(xref_t *xref);

sigil_err_t read_startxref(sigil_t *sgl);

sigil_err_t read_xref_table(sigil_t *sgl);

sigil_err_t process_xref(sigil_t *sgl);

void print_xref(xref_t *xref);

int sigil_xref_self_test(int verbosity);

#endif /* PDF_SIGIL_XREF_H */
