#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "sigil.h"


static sigil_err_t validate_mode(mode_t mode)
{
    if (mode != MODE_VERIFY && mode != MODE_SIGN) {
        return (sigil_err_t)ERR_PARAM;
    }

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_init(sigil_t **sgl)
{
    // function parameter checks
    if (sgl == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    *sgl = malloc(sizeof(sigil_t));

    if (*sgl == NULL) {
        return (sigil_err_t)ERR_ALLOC;
    }

    // set default values
    (*sgl)->file             = NULL;
    (*sgl)->filepath         = NULL;
    (*sgl)->mode             = MODE_UNSET;
    (*sgl)->pdf_x            = 0;
    (*sgl)->pdf_y            = 0;
    (*sgl)->file_size        = 0;
    (*sgl)->pdf_start_offset = 0;
    (*sgl)->startxref        = 0;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_config_file(sigil_t *sgl, const char *filepath)
{
    // function parameter checks
    if (sgl == NULL || filepath == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    // get size of filepath
    size_t filepath_len = strlen(filepath);
    if (filepath_len <= 0) {
        return (sigil_err_t)ERR_PARAM;
    }

    // allocate space for copy of provided string
    sgl->filepath = malloc((filepath_len + 1) * sizeof(filepath[0]));
    if (sgl->filepath == NULL) {
        return (sigil_err_t)ERR_ALLOC;
    }

    // copy string filepath into sigil_t structure
    int written = snprintf(sgl->filepath, filepath_len + 1, filepath);
    if (written < 0 || written >= filepath_len + 1) {
        return (sigil_err_t)ERR_IO;
    }

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_config_mode(sigil_t *sgl, mode_t mode)
{
    // function parameter checks
    if (sgl == NULL || validate_mode(mode) != ERR_NO) {
        return (sigil_err_t)ERR_PARAM;
    }

    sgl->mode = mode;

    return (sigil_err_t)ERR_NO;
}

static sigil_err_t process_trailer(sigil_t *sgl)
{
    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    // TODO
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

    if (sgl->file == NULL) {
        return (sigil_err_t)ERR_IO;
    }

    // process header - %PDF-<pdf_x>.<pdf_y>
    err = process_header(sgl);
    if (err != ERR_NO) {
        return err;
    }

    // process trailer
    err = process_trailer(sgl);
    if (err != ERR_NO) {
        return err;
    }

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

int sigil_sigil_self_test(int quiet)
{
    if (!quiet)
        printf("\n + Testing module: sigil\n");

    // TEST: fn validate_mode
    if (!quiet)
        printf("    - %-30s", "fn validate_mode");

    if (validate_mode(MODE_UNSET)  != ERR_PARAM ||
        validate_mode(MODE_VERIFY) != ERR_NO    ||
        validate_mode(MODE_SIGN)   != ERR_NO    ||
        validate_mode(0xffff)      != ERR_PARAM )
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: fn sigil_init
    if (!quiet)
        printf("    - %-30s", "fn sigil_init");

    sigil_t *sgl = NULL;
    sigil_err_t err = sigil_init(&sgl);
    if (err != ERR_NO || sgl == NULL) {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: fn sigil_config_file
    if (!quiet)
        printf("    - %-30s", "fn sigil_config_file");

    err = sigil_config_file(sgl, "test/uznavany_bez_razitka_bez_revinfo_27_2_2012_CMS.pdf");
    if (err != ERR_NO || sgl->filepath == NULL) {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: fn sigil_config_mode
    if (!quiet)
        printf("    - %-30s", "fn sigil_config_mode");

    err = sigil_config_mode(sgl, 0xffff);
    if (err != ERR_PARAM) {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    err = sigil_config_mode(sgl, MODE_VERIFY);
    if (err != ERR_NO) {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: fn process_header
    if (!quiet)
        printf("    - %-30s", "fn process_header");

    // prepare
    sgl->file = fopen(sgl->filepath, "r");

    if (sgl->file == NULL) {
        if (!quiet)
            printf("TEST PROCEDURE FAILED\n");

        goto failed;
    }

    err = process_header(sgl);
    if (err != ERR_NO || sgl->pdf_x != 1 || sgl->pdf_y != 3 ||
        sgl->pdf_start_offset != 0)
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    sigil_free(sgl);

    // all tests done
    if (!quiet) {
        printf("   PASSED\n");
        fflush(stdout);
    }

    return 0;

failed:
    if (!quiet) {
        printf("   FAILED\n");
        fflush(stdout);
    }

    return 1;
}
