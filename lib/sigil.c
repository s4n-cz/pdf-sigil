#include <stdio.h>
#include <stdlib.h>
#include "auxiliary.h"
#include "constants.h"
#include "header.h"
#include "sigil.h"
#include "trailer.h"
#include "xref.h"

sigil_err_t sigil_init(sigil_t **sgl)
{
    // function parameter checks
    if (sgl == NULL)
        return ERR_PARAM;

    *sgl = malloc(sizeof(sigil_t));

    if (*sgl == NULL)
        return ERR_ALLOC;

    sigil_zeroize(*sgl, sizeof(*sgl));

    // set default values
    (*sgl)->file                            = NULL;
    (*sgl)->pdf_x                           = 0;
    (*sgl)->pdf_y                           = 0;
    (*sgl)->xref_type                       = XREF_TYPE_UNSET;
    (*sgl)->xref                            = NULL;
    (*sgl)->ref_catalog_dict.object_num     = 0;
    (*sgl)->ref_catalog_dict.generation_num = 0;
    (*sgl)->file_size                       = 0;
    (*sgl)->pdf_start_offset                = 0;
    (*sgl)->startxref                       = 0;

    return ERR_NO;
}

sigil_err_t sigil_verify(sigil_t *sgl, const char *filepath)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL || filepath == NULL)
        return ERR_PARAM;

    // open provided file
    if ((sgl->file = fopen(filepath, "r")) == NULL)
        return ERR_IO;

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

    return ERR_NO;
}

void sigil_free(sigil_t *sgl)
{
    if (sgl == NULL)
        return;
    if (sgl->file)
        fclose(sgl->file);
    if (sgl->xref)
        xref_free(sgl->xref);
    free(sgl);
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
