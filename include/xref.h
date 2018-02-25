#ifndef PDF_SIGIL_XREF_H
#define PDF_SIGIL_XREF_H

#include "error.h"
#include "sigil.h"


#ifndef XREF_T
#define XREF_T

typedef struct {
    size_t byte_offset;
    size_t generation_num;
} xref_entry_t;

typedef struct {
    xref_entry_t **entry;
    size_t capacity;
} xref_t;

#endif /* XREF_T */

xref_t *new_xref();

void free_xref(xref_t *xref);

sigil_err_t read_startxref(sigil_t *sgl);

sigil_err_t read_xref_table(sigil_t *sgl);

sigil_err_t process_xref(sigil_t *sgl);

void print_xref(xref_t *xref);

int sigil_xref_self_test(int verbosity);

#endif /* PDF_SIGIL_XREF_H */
