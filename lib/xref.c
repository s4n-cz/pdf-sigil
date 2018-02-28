#if 0
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "config.h"
#include "error.h"
#include "sigil.h"
#include "xref.h"
#endif

#include <stdlib.h>
#include <string.h>
#include "auxiliary.h"
#include "config.h"
#include "constants.h"
#include "sigil.h"
#include "xref.h"

// Determine whether this file is using Cross-reference table or stream
static sigil_err_t determine_xref_type(sigil_t *sgl)
{
    if (fseek(sgl->file, sgl->startxref, SEEK_SET) != 0)
        return ERR_IO;

    char_t c = fgetc(sgl->file);
    if (c == 'x') {
        sgl->xref_type = XREF_TYPE_TABLE;
    } else if (is_digit(c)) {
        sgl->xref_type = XREF_TYPE_STREAM;
    } else {
        return ERR_PDF_CONT;
    }

    return ERR_NO;
}

static sigil_err_t
add_xref_entry(xref_t *xref, size_t obj, size_t offset, size_t generation)
{
    int resize_factor = 1;

    if (xref == NULL)
        return ERR_PARAM;

    // resize if needed
    while (obj > resize_factor * xref->capacity - 1)
        resize_factor *= 2;
    if (resize_factor != 1) {
        xref->entry = realloc(xref->entry,sizeof(xref_entry_t *) * xref->capacity * resize_factor);
        if (xref->entry == NULL)
            return ERR_ALLOC;
        sigil_zeroize(xref->entry + xref->capacity, sizeof(xref_entry_t *) * (xref->capacity * (resize_factor - 1)));
        xref->capacity *= resize_factor;
    }

    if (xref->entry[obj] != NULL) {
        if (xref->entry[obj]->generation_num < generation) {
            xref->entry[obj]->byte_offset = offset;
            xref->entry[obj]->generation_num = generation;
        } else {
            return ERR_NO;
        }
    } else {
        xref->entry[obj] = malloc(sizeof(xref_entry_t));
        if (xref->entry[obj] == NULL)
            return ERR_ALLOC;
        sigil_zeroize(xref->entry[obj], sizeof(xref->entry[obj]));

        xref->entry[obj]->byte_offset = offset;
        xref->entry[obj]->generation_num = generation;
    }

    return ERR_ALLOC;
}

static void free_xref_entry(xref_entry_t *entry)
{
    if (entry != NULL)
        free(entry);
}

xref_t *xref_init()
{
    xref_t *xref = malloc(sizeof(xref_t));
    if (xref == NULL)
        return NULL;
    sigil_zeroize(xref, sizeof(xref));

    xref->entry = malloc(sizeof(xref_entry_t *) * XREF_PREALLOCATION);
    if (xref->entry == NULL) {
        free(xref);
        return NULL;
    }
    sigil_zeroize(xref->entry, sizeof(*(xref->entry)) * xref->capacity);
    xref->capacity = XREF_PREALLOCATION;
    xref->size_from_trailer = 0;
    xref->prev_section = 0;

    return xref;
}

void xref_free(xref_t *xref)
{
    if (xref == NULL)
        return;

    if (xref->entry != NULL) {
        for (int i = 0; i < xref->capacity; i++) {
            free_xref_entry(xref->entry[i]);
        }
        free(xref->entry);
    }

    free(xref);
}

sigil_err_t read_startxref(sigil_t *sgl)
{
    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return ERR_PARAM;
    }

    // jump to the end of file
    if (fseek(sgl->file, 0, SEEK_END) != 0) {
        return ERR_IO;
    }

    // get file size
    if (sgl->file_size <= 0) {
        sgl->file_size = ftell(sgl->file);
        if (sgl->file_size < 0) {
            return ERR_IO;
        }
    }

    // jump max XREF_SEARCH_OFFSET bytes from end
    size_t jump_pos = MAX(0, (ssize_t)sgl->file_size - XREF_SEARCH_OFFSET);
    if (fseek(sgl->file, jump_pos, SEEK_SET) != 0) {
        return ERR_IO;
    }

    // prepare buffer for data
    size_t buf_len = sgl->file_size - jump_pos + 1;
    char_t *buf = malloc(buf_len * sizeof(char_t));
    if (buf == NULL)
        return ERR_ALLOC;
    sigil_zeroize(buf, buf_len * sizeof(*buf));

    // copy data from the end of file
    size_t read = fread(buf, sizeof(*buf), buf_len - 1, sgl->file);
    if (read <= 0) {
        free(buf);
        return ERR_IO;
    }
    buf[read] = '\0';

    // search for 'startxref' from the end
    for (int i = read - 9; i >= 0; i--) {
        if (memcmp(buf + i, "startxref", 9) == 0) {
            i += 9;
            while (i < read && is_whitespace(buf[i])) {
                i++;
            }
            if (!is_digit(buf[i])) {
                free(buf);
                return ERR_PDF_CONT;
            }
            sgl->startxref = 0;
            while (i < read && is_digit(buf[i])) {
                sgl->startxref = 10 * sgl->startxref + buf[i] - '0';
                i++;
            }
            break;
        }
    }

    free(buf);

    if (sgl->startxref == 0)
        return ERR_PDF_CONT;

    return ERR_NO;
}

sigil_err_t read_xref_table(sigil_t *sgl)
{
    char tmp[5];
    free_indicator_t free_indicator;
    size_t section_start = 0,
           section_cnt = 0,
           obj_offset,
           obj_generation;
    int xref_end = 0;
    sigil_err_t err;

    if (sgl->xref == NULL)
        sgl->xref = xref_init();
    if (sgl->xref == NULL)
        return ERR_ALLOC;

    if (fseek(sgl->file, sgl->startxref, SEEK_SET) != 0)
        return ERR_IO;

    // read "xref"
    if (fgets(tmp, 5, sgl->file) == NULL)
        return ERR_IO;

    if (strncmp(tmp, "xref", 4) != 0)
        return ERR_PDF_CONT;

    while (!xref_end) { // for all xref sections
        while (1) {
            // read 2 numbers:
            //     - first object in subsection
            //     - number of entries in subsection
            if (parse_number(sgl->file, &section_start) != ERR_NO) {
                xref_end = 1;
                break;
            }
            if (parse_number(sgl->file, &section_cnt) != ERR_NO)
                return 1;
            if (section_start < 0 || section_cnt < 1)
                return 1;

            // for all entries in one section
            for (int section_offset = 0; section_offset < section_cnt; section_offset++) {
                err = parse_number(sgl->file, &obj_offset);
                if (err != ERR_NO)
                    return err;
                err = parse_number(sgl->file, &obj_generation);
                if (err != ERR_NO)
                    return err;
                err = parse_free_indicator(sgl->file, &free_indicator);
                if (err != ERR_NO)
                    return err;
                size_t obj_num = section_start + section_offset;
                if (free_indicator == IN_USE_ENTRY) {
                    err = add_xref_entry(sgl->xref, obj_num, obj_offset, obj_generation); if (err != ERR_NO)
                        return err;
                }
            }
        }
    }
    return ERR_NO;
}

sigil_err_t process_xref(sigil_t *sgl)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL || sgl->file == NULL || sgl->startxref == 0) {
        return ERR_PARAM;
    }

    err = read_startxref(sgl);
    if (err != ERR_NO)
        return err;

    err = determine_xref_type(sgl);
    if (err != ERR_NO)
        return err;

    switch (sgl->xref_type) {
        case XREF_TYPE_TABLE:
            read_xref_table(sgl);
            break;
        case XREF_TYPE_STREAM:
            return ERR_NOT_IMPL; // TODO
        default:
            return ERR_PDF_CONT;
    }

    return ERR_NO;
}

void print_xref(xref_t *xref)
{
    if (xref == NULL)
        return;

    printf("\nXREF\n");
    for (int i = 0; i < xref->capacity; i++) {
        if (xref->entry[i] != NULL)
            printf("obj %d | offset %zd | generation %zd\n", i, xref->entry[i]->byte_offset, xref->entry[i]->generation_num);
    }
}

int sigil_xref_self_test(int verbosity)
{
    sigil_err_t err;
    sigil_t *sgl = NULL;

    print_module_name("xref", verbosity);

    // TEST: fn determine_xref_type - TABLE
    print_test_item("fn determine_xref_type TABLE", verbosity);

    {
        sgl = NULL;
        err = sigil_init(&sgl);
        if (err != ERR_NO || sgl == NULL)
            goto failed;

        sgl->file = fopen("test/uznavany_bez_razitka_bez_revinfo_27_2_2012_CMS.pdf", "r");
        if (sgl->file == NULL)
            goto failed;
        sgl->xref_type = XREF_TYPE_UNSET;
        sgl->startxref = 67954;

        if (determine_xref_type(sgl) != ERR_NO ||
            sgl->xref_type != XREF_TYPE_TABLE)
        {
            goto failed;
        }

        sigil_free(sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn determine_xref_type - STREAM
    print_test_item("fn determine_xref_type STREAM", verbosity);

    {
        sgl = NULL;
        err = sigil_init(&sgl);
        if (err != ERR_NO || sgl == NULL)
            goto failed;

        sgl->file = fopen("test/SampleSignedPDFDocument.pdf", "r");
        if (sgl->file == NULL)
            goto failed;
        sgl->xref_type = XREF_TYPE_UNSET;
        sgl->startxref = 116;

        if (determine_xref_type(sgl) != ERR_NO ||
            sgl->xref_type != XREF_TYPE_STREAM)
        {
            goto failed;
        }

        sigil_free(sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn read_startxref
    print_test_item("fn read_startxref", verbosity);

    {
        sgl = NULL;
        char *sstream_1 = "startxref\n" \
                          "1234567890\n"\
                          "\045\045EOF"; // %%EOF

        err = sigil_init(&sgl);
        if (err != ERR_NO || sgl == NULL)
            goto failed;

        sgl->file = fmemopen(sstream_1,
                             (strlen(sstream_1) + 1) * sizeof(*sstream_1),
                             "r");
        if (sgl->file == NULL)
            goto failed;

        if (read_startxref(sgl) != ERR_NO ||
            sgl->file_size != 27          ||
            sgl->startxref != 1234567890)
        {
            goto failed;
        }

        sigil_free(sgl);
    }

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    if (sgl)
        sigil_free(sgl);

    print_test_result(0, verbosity);
    print_module_result(0, verbosity);
    return 1;
}
