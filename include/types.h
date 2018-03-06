#ifndef PDF_SIGIL_TYPES_H
#define PDF_SIGIL_TYPES_H

#include <stdint.h> // uint32_t


typedef uint32_t sigil_err_t;

typedef uint32_t keyword_t;

typedef uint32_t free_indicator_t;

typedef uint32_t dict_key_t;

typedef struct {
    size_t object_num;
    size_t generation_num;
} reference_t;

typedef struct {
    size_t byte_offset;
    size_t generation_num;
} xref_entry_t;

typedef struct {
    xref_entry_t **entry;
    size_t         capacity;
    size_t         size_from_trailer;
    size_t         prev_section;
} xref_t;

typedef struct {
    FILE    *file;
    char    *buffer;
    size_t   buf_pos;
    size_t   size;
    uint32_t deallocation_info;
} pdf_data_t;

typedef struct {
    pdf_data_t  pdf_data;
    short       pdf_x,             /* numbers from PDF header */
                pdf_y;             /*   %PDF-<pdf_x>.<pdf_y>  */
    short       xref_type;
    xref_t     *xref;
    reference_t ref_catalog_dict;
    size_t      pdf_start_offset;  /* offset of %PDF-x.y      */
    size_t      startxref;
} sigil_t;

#endif /* PDF_SIGIL_TYPES_H */
