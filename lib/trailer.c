#include "auxiliary.h"
#include "xref.h"
#include "sigil.h"

// Determine whether this file is using Cross-reference table or stream
static sigil_err_t determine_xref_type(sigil_t *sgl)
{
    if (fseek(sgl->file, sgl->startxref, SEEK_SET) != 0)
        return (sigil_err_t)ERR_IO;

    char c = fgetc(sgl->file);
    if (c == 'x') {
        sgl->xref_type = XREF_TYPE_TABLE;
    } else if (is_digit(c)) {
        sgl->xref_type = XREF_TYPE_STREAM;
    } else {
        return (sigil_err_t)ERR_PDF_CONT;
    }

    return (sigil_err_t)ERR_NO;
}

sigil_err_t process_trailer(sigil_t *sgl)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    err = read_startxref(sgl);
    if (err != ERR_NO)
        return err;

    err = determine_xref_type(sgl);
    if (err != ERR_NO)
        return err;

    // TODO

    return 0;
}

int sigil_trailer_self_test(int verbosity)
{
    print_module_name("trailer", verbosity);

    // TEST: fn determine_xref_type - TABLE
    print_test_item("fn determine_xref_type 1", verbosity);

    sigil_t *sgl = malloc(sizeof(sigil_t));
    if (sgl == NULL)
        return (sigil_err_t)ERR_ALLOC;
    sgl->file = fopen("test/uznavany_bez_razitka_bez_revinfo_27_2_2012_CMS.pdf", "r");
    if (sgl->file == NULL)
        return (sigil_err_t)ERR_IO;
    sgl->xref_type = XREF_TYPE_UNSET;
    sgl->startxref = 67954;

    if (determine_xref_type(sgl) != ERR_NO ||
        sgl->xref_type != XREF_TYPE_TABLE)
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    fclose(sgl->file);

    // TEST: fn determine_xref_type - STREAM
    print_test_item("fn determine_xref_type 2", verbosity);

    sgl->file = fopen("test/SampleSignedPDFDocument.pdf", "r");
    if (sgl->file == NULL)
        return (sigil_err_t)ERR_IO;
    sgl->xref_type = XREF_TYPE_UNSET;
    sgl->startxref = 116;

    if (determine_xref_type(sgl) != ERR_NO ||
        sgl->xref_type != XREF_TYPE_STREAM)
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    fclose(sgl->file);
    free(sgl);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    print_test_result(0, verbosity);
    print_module_result(0, verbosity);
    return 1;
}
