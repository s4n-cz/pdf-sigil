#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
    (*sgl)->pdf_start_offset = 0;

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
    int ret_size = snprintf(sgl->filepath, filepath_len + 1, filepath);
    if (ret_size < 0 || ret_size >= filepath_len + 1) {
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

static sigil_err_t parse_header(sigil_t *sgl)
{
    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    const char expected[] = {'%', 'P', 'D', 'F', '-'};
    size_t offset = 0;
    int found = 0,
        c;

    while ((c = fgetc(sgl->file) != EOF)) {
        // count offset from start to '%' char
        // PDF header size is subtracted later
        offset++;

        if (found < 5) {
            if (c == (int)expected[found]) {
                found++;
            } else if (c == (int)expected[0]) {
                found = 1;
            } else {
                found = 0;
            }
        } else if (found == 5) {
            if (c >= (int)'0' && c <= (int)'9') {
                sgl->pdf_x = c - '0';
                found++;
            } else if (c == (int)expected[0]) {
                found = 1;
            } else {
                found = 0;
            }
        } else if (found == 6) {
            if (c == (int)'.') {
                found++;
            } else if (c == (int)expected[0]) {
                found = 1;
            } else {
                found = 0;
            }
        } else if (found == 7) {
            if (c >= (int)'0' && c <= (int)'9') {
                sgl->pdf_y = c - '0';
                found++;
            } else if (c == (int)expected[0]) {
                found = 1;
            } else {
                found = 0;
            }
        }
    }

    if (found != 8) {
        return (sigil_err_t)ERR_PDF_CONT;
    }

    // offset counted from % char -> subtract header size
    sgl->pdf_start_offset = offset - 8;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_process(sigil_t *sgl)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL || sgl->filepath == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }
    if (validate_mode(sgl->mode) != ERR_NO) {
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
    err = parse_header(sgl);
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
        validate_mode(0xffff)      != ERR_PARAM  )
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
