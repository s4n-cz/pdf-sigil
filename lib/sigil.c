#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "error.h"
#include "header.h"
#include "sigil.h"
#include "trailer.h"
#include "xref.h"


sigil_err_t sigil_init(sigil_t **sgl)
{
    // function parameter checks
    if (sgl == NULL)
        return (sigil_err_t)ERR_PARAM;

    *sgl = malloc(sizeof(sigil_t));

    if (*sgl == NULL)
        return (sigil_err_t)ERR_ALLOC;

    // set default values
    (*sgl)->file             = NULL;
    (*sgl)->filepath         = NULL;
    (*sgl)->pdf_x            = 0;
    (*sgl)->pdf_y            = 0;
    (*sgl)->xref_type        = XREF_TYPE_UNSET;
    (*sgl)->xref             = NULL;
    (*sgl)->file_size        = 0;
    (*sgl)->pdf_start_offset = 0;
    (*sgl)->startxref        = 0;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_setup_file(sigil_t *sgl, const char *filepath)
{
    // function parameter checks
    if (sgl == NULL || filepath == NULL)
        return (sigil_err_t)ERR_PARAM;

    // get size of filepath
    size_t filepath_len = strlen(filepath);
    if (filepath_len <= 0)
        return (sigil_err_t)ERR_PARAM;

    // allocate space for copy of provided string
    sgl->filepath = malloc((filepath_len + 1) * sizeof(char_t));
    if (sgl->filepath == NULL)
        return (sigil_err_t)ERR_ALLOC;

    // copy string filepath into sigil_t structure
    int written = snprintf(sgl->filepath, filepath_len + 1, filepath);
    if (written < 0 || written >= filepath_len + 1)
        return (sigil_err_t)ERR_IO;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_verify(sigil_t *sgl)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL || sgl->filepath == NULL)
        return (sigil_err_t)ERR_PARAM;

    // open provided file
    if ((sgl->file = fopen(sgl->filepath, "r")) == NULL)
        return (sigil_err_t)ERR_IO;

    // process header - %PDF-<pdf_x>.<pdf_y>
    err = process_header(sgl);
    if (err != ERR_NO)
        return err;

    // process cross-reference section
    err = process_xref(sgl);
    if (err != ERR_NO)
        return err;

    err = process_trailer(sgl);
    if (err != ERR_NO)
        return err;

    // TODO

    return (sigil_err_t)ERR_NO;
}

void sigil_free(sigil_t *sgl)
{
    if (sgl) {
        if (sgl->file)
            fclose(sgl->file);
        if (sgl->filepath)
            free(sgl->filepath);
        if (sgl->xref)
            xref_free(sgl->xref);
        free(sgl);
        sgl = NULL;
    }
}

int sigil_sigil_self_test(int verbosity)
{
    sigil_err_t err;
    sigil_t *sgl = NULL;

    print_module_name("sigil", verbosity);

    // TEST: fn sigil_init
    print_test_item("fn sigil_init", verbosity);

    {
        sgl = NULL;
        err = sigil_init(&sgl);
        if (err != ERR_NO || sgl == NULL)
            goto failed;

        sigil_free(sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn sigil_setup_file
    print_test_item("fn sigil_setup_file", verbosity);

    {
        sgl = NULL;
        err = sigil_init(&sgl);
        if (err != ERR_NO || sgl == NULL)
            goto failed;

        err = sigil_setup_file(sgl, "test/uznavany_bez_razitka_bez_revinfo_27_2_2012_CMS.pdf");
        if (err != ERR_NO || sgl->filepath == NULL)
            goto failed;

        sigil_free(sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn sigil_verify
    print_test_item("fn sigil_verify", verbosity);

    {
        sgl = NULL;
        err = sigil_init(&sgl);
        if (err != ERR_NO || sgl == NULL)
            goto failed;

        // TODO
        if (1)
            goto failed;

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
