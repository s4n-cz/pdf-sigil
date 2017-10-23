#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sigil.h"


sigil_err_t sigil_init(sigil_t **sgl)
{
    *sgl = malloc(sizeof(sigil_t));

    if (*sgl == NULL) {
        return (sigil_err_t)ERR_ALLOC;
    }

    // set default values
    (*sgl)->file     = NULL;
    (*sgl)->filename = NULL;
    (*sgl)->mode     = ERR_NO;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t sigil_config_file(sigil_t *sgl, const char *filename)
{
    // input parameter checks - filename
    if (filename == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    size_t filename_len = strlen(filename);
    if (filename_len <= 0) {
        return (sigil_err_t)ERR_PARAM;
    }

    // allocate space for copy of provided string
    sgl->filename = malloc((filename_len + 1) * sizeof(filename[0]));
    if (sgl->filename == NULL) {
        return (sigil_err_t)ERR_ALLOC;
    }

    // copy string filename into sigil_t structure
    int ret_size = snprintf(sgl->filename, filename_len + 1, filename);
    if (ret_size < 0 || ret_size >= filename_len + 1) {
        return (sigil_err_t)ERR_IO;
    }

    return (sigil_err_t)ERR_NO;
}

void sigil_config_mode(sigil_t *sgl, uint32_t mode)
{
    sgl->mode = mode;
}

sigil_err_t sigil_process(sigil_t *sgl)
{

}

void sigil_free(sigil_t *sgl)
{
    if (sgl) {
        if (sgl->file) {
            fclose(sgl->file);
        }
        if (sgl->filename) {
            free(sgl->filename);
        }
        free(sgl);
        sgl = NULL;
    }
}
