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
    sigil_err_t err;
    char c;

    err = pdf_move_pos_abs(sgl, sgl->startxref);
    if (err != ERR_NO)
        return err;

    if ((err = pdf_peek_char(sgl, &c)) != ERR_NO)
        return err;

    if (c == 'x') {
        sgl->xref_type = XREF_TYPE_TABLE;
    } else if (is_digit(c)) {
        sgl->xref_type = XREF_TYPE_STREAM;
    } else {
        return ERR_PDF_CONTENT;
    }

    return ERR_NO;
}

static sigil_err_t
add_xref_entry(xref_t *xref, size_t obj, size_t offset, size_t generation)
{
    int resize_factor = 1;

    if (xref == NULL)
        return ERR_PARAMETER;

    // resize if needed
    while (obj > resize_factor * xref->capacity - 1) {
        resize_factor *= 2;
    }

    if (resize_factor != 1) {
        xref->entry = realloc(xref->entry,sizeof(xref_entry_t *) * xref->capacity * resize_factor);
        if (xref->entry == NULL)
            return ERR_ALLOCATION;
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
            return ERR_ALLOCATION;
        sigil_zeroize(xref->entry[obj], sizeof(xref->entry[obj]));

        xref->entry[obj]->byte_offset = offset;
        xref->entry[obj]->generation_num = generation;
    }

    return ERR_ALLOCATION;
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
    sigil_err_t err;
    size_t file_size;
    size_t offset;
    size_t read_size;
    char tmp[10];

    if (sgl == NULL)
        return ERR_PARAMETER;

    file_size = sgl->pdf_data.size - 1;

    err = pdf_move_pos_abs(sgl, file_size - 9);
    if (err != ERR_NO)
        return err;
    offset = 9;

    while (1) {
        if (offset > XREF_SEARCH_OFFSET)
            return ERR_PDF_CONTENT;

        err = pdf_read(sgl, 9, tmp, &read_size);
        if (err != ERR_NO)
            return err;
        if (read_size != 9)
            return ERR_PDF_CONTENT;

        if (strncmp(tmp, "startxref", 9) == 0) {
            if ((err = parse_number(sgl, &(sgl->startxref))) != ERR_NO)
                return err;
            if (sgl->startxref == 0)
                return ERR_PDF_CONTENT;

            return ERR_NO;
        }

        if ((err = pdf_move_pos_rel(sgl, -10)) != ERR_NO)
            return err;
        offset++;
    }

    return ERR_PDF_CONTENT;
}

sigil_err_t read_xref_table(sigil_t *sgl)
{
    free_indicator_t free_indicator;
    size_t section_start = 0,
           section_cnt = 0,
           obj_offset,
           obj_generation;
    int xref_end = 0;
    keyword_t keyword;
    sigil_err_t err;

    if (sgl->xref == NULL)
        sgl->xref = xref_init();
    if (sgl->xref == NULL)
        return ERR_ALLOCATION;

    err = pdf_move_pos_abs(sgl, sgl->startxref);
    if (err != ERR_NO)
        return err;

    // read "xref"
    if ((err = parse_keyword(sgl, &keyword)) != ERR_NO)
        return err;
    if (keyword != KEYWORD_xref)
        return ERR_PDF_CONTENT;

    while (!xref_end) { // for all xref sections
        while (1) {
            // read 2 numbers:
            //     - first object in subsection
            //     - number of entries in subsection
            if (parse_number(sgl, &section_start) != ERR_NO) {
                xref_end = 1;
                break;
            }
            if (parse_number(sgl, &section_cnt) != ERR_NO)
                return 1;
            if (section_start < 0 || section_cnt < 1)
                return 1;

            // for all entries in one section
            for (int section_offset = 0; section_offset < section_cnt; section_offset++) {
                err = parse_number(sgl, &obj_offset);
                if (err != ERR_NO)
                    return err;
                err = parse_number(sgl, &obj_generation);
                if (err != ERR_NO)
                    return err;
                err = parse_free_indicator(sgl, &free_indicator);
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

    if (sgl == NULL || sgl->startxref == 0)
        return ERR_PARAMETER;

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
            return ERR_NOT_IMPLEMENTED; // TODO
        default:
            return ERR_PDF_CONTENT;
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
    sigil_t *sgl = NULL;

    print_module_name("xref", verbosity);

    // TEST: fn determine_xref_type - TABLE
    print_test_item("fn determine_xref_type TABLE", verbosity);

    {
        sgl = test_prepare_sgl_path(
            "test/uznavany_bez_razitka_bez_revinfo_27_2_2012_CMS.pdf");
        if (sgl == NULL)
            goto failed;

        sgl->xref_type = XREF_TYPE_UNSET;
        sgl->startxref = 67954;

        if (determine_xref_type(sgl) != ERR_NO ||
            sgl->xref_type != XREF_TYPE_TABLE)
        {
            goto failed;
        }

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn determine_xref_type - STREAM
    print_test_item("fn determine_xref_type STREAM", verbosity);

    {
        sgl = test_prepare_sgl_path(
            "test/SampleSignedPDFDocument.pdf");
        if (sgl == NULL)
            goto failed;

        sgl->xref_type = XREF_TYPE_UNSET;
        sgl->startxref = 116;

        if (determine_xref_type(sgl) != ERR_NO ||
            sgl->xref_type != XREF_TYPE_STREAM)
        {
            goto failed;
        }

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn read_startxref
    print_test_item("fn read_startxref", verbosity);

    {
        char *sstream = "abcdefghi\n" \
                        "startxref\n" \
                        "1234567890\n"\
                        "\045\045EOF"; // %%EOF
        if ((sgl = test_prepare_sgl_content(sstream, strlen(sstream) + 1)) == NULL)
            goto failed;

        if (read_startxref(sgl) != ERR_NO ||
            sgl->startxref != 1234567890)
        {
            goto failed;
        }

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    if (sgl)
        sigil_free(&sgl);

    print_test_result(0, verbosity);
    print_module_result(0, verbosity);

    return 1;
}
