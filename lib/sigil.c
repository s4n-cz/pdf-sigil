#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "header.h"
#include "sigil.h"
#include "trailer.h"
#include "xref.h"


static sigil_err_t validate_mode(mode_t mode)
{
    if (mode != MODE_VERIFY && mode != MODE_SIGN)
        return (sigil_err_t)ERR_PARAM;

    return (sigil_err_t)ERR_NO;
}

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
    (*sgl)->mode             = MODE_UNSET;
    (*sgl)->pdf_x            = 0;
    (*sgl)->pdf_y            = 0;
    (*sgl)->xref_type        = XREF_TYPE_UNSET;
    (*sgl)->file_size        = 0;
    (*sgl)->pdf_start_offset = 0;
    (*sgl)->startxref        = 0;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_setup_file(sigil_t *sgl, const char_t *filepath)
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

sigil_err_t sigil_setup_mode(sigil_t *sgl, mode_t mode)
{
    // function parameter checks
    if (sgl == NULL || validate_mode(mode) != ERR_NO)
        return (sigil_err_t)ERR_PARAM;

    sgl->mode = mode;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_process(sigil_t *sgl)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL || sgl->filepath == NULL ||
        validate_mode(sgl->mode) != ERR_NO   )
    {
        return (sigil_err_t)ERR_PARAM;
    }

    // open provided file
    if (sgl->mode == MODE_VERIFY) {
        sgl->file = fopen(sgl->filepath, "r");
    } else if (sgl->mode == MODE_SIGN) {
        sgl->file = fopen(sgl->filepath, "r+");
    }

    if (sgl->file == NULL)
        return (sigil_err_t)ERR_IO;

    // process header - %PDF-<pdf_x>.<pdf_y>
    err = process_header(sgl);
    if (err != ERR_NO)
        return err;

    // process trailer
    err = process_trailer(sgl);
    if (err != ERR_NO)
        return err;

    // TODO

    return (sigil_err_t)ERR_NO;
}

void sigil_free(sigil_t *sgl)
{
    if (sgl) {
        if (sgl->file) {
            fclose(sgl->file);
        }
        if (sgl->filepath) {
            free(sgl->filepath);
        }
        free(sgl);
        sgl = NULL;
    }
}

int sigil_sigil_self_test(int verbosity)
{
    print_module_name("sigil", verbosity);

    // TEST: fn validate_mode
    print_test_item("fn validate_mode", verbosity);

    if (validate_mode(MODE_UNSET)  != ERR_PARAM ||
        validate_mode(MODE_VERIFY) != ERR_NO    ||
        validate_mode(MODE_SIGN)   != ERR_NO    ||
        validate_mode(0xffff)      != ERR_PARAM )
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    // TEST: fn sigil_init
    print_test_item("fn sigil_init", verbosity);

    sigil_t *sgl = NULL;
    sigil_err_t err = sigil_init(&sgl);
    if (err != ERR_NO || sgl == NULL)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: fn sigil_setup_file
    print_test_item("fn sigil_setup_file", verbosity);

    err = sigil_setup_file(sgl, "test/uznavany_bez_razitka_bez_revinfo_27_2_2012_CMS.pdf");
    if (err != ERR_NO || sgl->filepath == NULL)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: fn sigil_setup_mode
    print_test_item("fn sigil_setup_mode", verbosity);

    err = sigil_setup_mode(sgl, 0xffff);
    if (err != ERR_PARAM)
        goto failed;

    err = sigil_setup_mode(sgl, MODE_VERIFY);
    if (err != ERR_NO)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: fn process_header
    print_test_item("fn process_header", verbosity);

    // prepare
    sgl->file = fopen(sgl->filepath, "r");

    if (sgl->file == NULL)
        goto failed;

    err = process_header(sgl);
    if (err != ERR_NO || sgl->pdf_x != 1 || sgl->pdf_y != 3 ||
        sgl->pdf_start_offset != 0)
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    sigil_free(sgl);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    print_test_result(0, verbosity);
    print_module_result(0, verbosity);
    return 1;
}
