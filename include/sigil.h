#ifndef PDF_SIGIL_SIGIL_H
#define PDF_SIGIL_SIGIL_H

#include <stdio.h>

#ifndef CHAR_T
#define CHAR_T
    typedef unsigned char char_t;
#endif /* CHAR_T */

#ifndef SIGIL_ERR_T
#define SIGIL_ERR_T
    typedef uint32_t sigil_err_t;
#endif /* SIGIL_ERR_T */

#ifndef XREF_T
#define XREF_T
    typedef struct {
        size_t byte_offset;
        size_t generation_num;
    } xref_entry_t;

    typedef struct {
        xref_entry_t **entry;
        size_t capacity;
        size_t size_from_trailer;
        size_t prev_section;
    } xref_t;
#endif /* XREF_T */

#ifndef REREFENCE_T
#define REREFENCE_T
    typedef struct {
        size_t object_num;
        size_t generation_num;
    } reference_t;
#endif /* REREFENCE_T */

#define XREF_TYPE_UNSET    0
#define XREF_TYPE_TABLE    1
#define XREF_TYPE_STREAM   2

typedef struct {
    FILE       *file;
    short       pdf_x,             /* numbers from PDF header */
                pdf_y;             /*   %PDF-<pdf_x>.<pdf_y>  */
    short       xref_type;
    xref_t     *xref;
    reference_t ref_catalog_dict;
    size_t      file_size;
    size_t      pdf_start_offset;  /* offset of %PDF-x.y      */
    size_t      startxref;
} sigil_t;

sigil_err_t sigil_init(sigil_t **sgl);

sigil_err_t sigil_verify(sigil_t *sgl, const char *filepath);

// ... get functions TODO

void sigil_free(sigil_t *sgl);

int sigil_sigil_self_test(int verbosity);

#endif /* PDF_SIGIL_SIGIL_H */
