#ifndef PDF_SIGIL_TYPES_H
#define PDF_SIGIL_TYPES_H

#include <openssl/evp.h> // EVP_MAX_MD_SIZE
#include <stdint.h> // uint32_t
#include <stdio.h>


#ifdef _WIN32
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif

typedef uint32_t sigil_err_t;

typedef uint32_t subfilter_t;

typedef uint32_t dict_key_t;

typedef struct {
    size_t object_num;
    size_t generation_num;
} reference_t;

typedef struct {
    char  *contents_hex;
    size_t capacity;
} contents_t;

typedef struct cert_t {
    char  *cert_hex;
    X509  *x509;
    size_t capacity;
    struct cert_t *next;
} cert_t;

typedef struct range_t {
    size_t start;
    size_t length;
    struct range_t *next;
} range_t;

typedef struct {
    reference_t **entry;
    size_t capacity;
} ref_array_t;

typedef struct xref_entry_t {
    size_t byte_offset;
    size_t generation_num;
    struct xref_entry_t *next;
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
    pdf_data_t     pdf_data;
    short          pdf_x,             /* numbers from PDF header */
                   pdf_y;             /*   %PDF-<pdf_x>.<pdf_y>  */
    short          xref_type;
    xref_t        *xref;
    reference_t    ref_catalog_dict;
    reference_t    ref_acroform;
    size_t         offset_acroform;
    reference_t    ref_sig_field;
    reference_t    ref_sig_dict;
    size_t         offset_sig_dict;
    ref_array_t    fields;
    size_t         pdf_start_offset;  /* offset of %PDF-x.y      */
    size_t         startxref;
    size_t         sig_flags;
    subfilter_t    subfilter;
    range_t       *byte_range;
    cert_t        *certificates;
    contents_t    *contents;
    unsigned char  computed_hash[EVP_MAX_MD_SIZE];
    unsigned int   computed_hash_len;
    int            signing_cert_status;
    X509_STORE    *trusted_store;
} sigil_t;

#endif /* PDF_SIGIL_TYPES_H */
