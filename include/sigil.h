#ifndef PDF_SIGIL_SIGIL_H
#define PDF_SIGIL_SIGIL_H

#include <stdio.h>
#include "error.h"

#ifndef CHAR_T
#define CHAR_T
    typedef char char_t;
#endif /* CHAR_T */

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

#define XREF_TYPE_UNSET    0
#define XREF_TYPE_TABLE    1
#define XREF_TYPE_STREAM   2

#define MODE_UNSET     0
#define MODE_VERIFY    1
#define MODE_SIGN      2

typedef uint32_t mode_t;
struct xref_t;

typedef struct {
    FILE   *file;
    char_t *filepath;
    mode_t  mode;
    short   pdf_x,             /* numbers from PDF header */
            pdf_y;             /*   %PDF-<pdf_x>.<pdf_y>  */
    short   xref_type;
    xref_t *xref;
    size_t  file_size;
    size_t  pdf_start_offset;  /* offset of %PDF-x.y      */
    size_t  startxref;
} sigil_t;

sigil_err_t sigil_init(sigil_t **sgl);

sigil_err_t sigil_setup_file(sigil_t *sgl, const char_t *filepath);

sigil_err_t sigil_setup_mode(sigil_t *sgl, mode_t mode);

sigil_err_t sigil_process(sigil_t *sgl);

// ... get functions TBD

void sigil_free(sigil_t *sgl);

int sigil_sigil_self_test(int verbosity);

#endif /* PDF_SIGIL_SIGIL_H */
