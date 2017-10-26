#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sigil.h"


sigil_err_t validate_mode(mode_t mode)
{
    if (mode != MODE_VERIFY &&
        mode != MODE_SIGN) {
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

sigil_err_t sigil_config_mode(sigil_t *sgl, uint32_t mode)
{
    // function parameter checks
    if (sgl == NULL || validate_mode(mode) != ERR_NO) {
        return (sigil_err_t)ERR_PARAM;
    }

    sgl->mode = mode;

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
