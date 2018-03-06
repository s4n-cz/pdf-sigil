#include <stdio.h>
#include <stdlib.h>
#include "auxiliary.h"
#include "config.h"
#include "constants.h"
#include "header.h"
#include "sigil.h"
#include "trailer.h"
#include "xref.h"

sigil_err_t sigil_init(sigil_t **sgl)
{
    // function parameter checks
    if (sgl == NULL)
        return ERR_PARAMETER;

    *sgl = malloc(sizeof(sigil_t));
    if (*sgl == NULL)
        return ERR_ALLOCATION;

    sigil_zeroize(*sgl, sizeof(*sgl));

    // set default values
    (*sgl)->pdf_data.file                   = NULL;
    (*sgl)->pdf_data.buffer                 = NULL;
    (*sgl)->pdf_data.buf_pos                = 0;
    (*sgl)->pdf_data.size                   = 0;
    (*sgl)->pdf_data.deallocation_info      = 0;
    (*sgl)->pdf_x                           = 0;
    (*sgl)->pdf_y                           = 0;
    (*sgl)->xref_type                       = XREF_TYPE_UNSET;
    (*sgl)->xref                            = NULL;
    (*sgl)->ref_catalog_dict.object_num     = 0;
    (*sgl)->ref_catalog_dict.generation_num = 0;
    (*sgl)->pdf_start_offset                = 0;
    (*sgl)->startxref                       = 0;

    return ERR_NO;
}

sigil_err_t sigil_set_pdf_file(sigil_t *sgl, FILE *pdf_file)
{
    size_t processed = 0;
    char *content = NULL;

    if (sgl == NULL || pdf_file == NULL)
        return ERR_PARAMETER;

    if (sgl->pdf_data.file != NULL && sgl->pdf_data.file != pdf_file)
        fclose(sgl->pdf_data.file);

    sgl->pdf_data.file = pdf_file;

    // get file size
    // - 1) jump to the end of file
    if (fseek(sgl->pdf_data.file, 0, SEEK_END) != 0)
        return ERR_IO;

    // - 2) read current position
    sgl->pdf_data.size = ftell(sgl->pdf_data.file) - 1;
    if (sgl->pdf_data.size < 0)
        return ERR_IO;

    // - 3) jump back to the beginning
    if (fseek(sgl->pdf_data.file, 0, SEEK_SET) != 0)
        return ERR_IO;

    if (sgl->pdf_data.size < THRESHOLD_FILE_BUFFERING) {
        content = malloc(sizeof(char) * (sgl->pdf_data.size + 1));
        if (content == NULL) {
            // fallback to using the file
            return ERR_NO;
        }

        processed = fread(content, sgl->pdf_data.size,
                          sizeof(char), sgl->pdf_data.file);
        if (processed != sgl->pdf_data.size) {
            // fallback to using the file
            free(content);
            return ERR_NO;
        }
        content[processed] = '\0';

        sgl->pdf_data.buffer = content;
        sgl->pdf_data.deallocation_info |= DEALLOCATE_BUFFER;
    }

    return ERR_NO;
}

sigil_err_t sigil_set_pdf_path(sigil_t *sgl, const char *path_to_pdf)
{
    if (sgl == NULL || path_to_pdf == NULL)
        return ERR_PARAMETER;

    FILE *pdf_file = NULL;

    #ifdef _WIN32
        // convert path to wchar_t
        size_t out_size;
        size_t path_len;
        wchar_t *path_to_pdf_win;

        path_len = strlen(path_to_pdf) + 1;
        path_to_pdf_win = malloc(path_len * sizeof(wchar_t));
        if (path_to_pdf_win == NULL)
            return ERR_ALLOCATION;
        sigil_zeroize(path_to_pdf_win, path_len * sizeof(wchar_t));
        if (mbstowcs_s(&out_size,       // out ... characters converted
                       path_to_pdf_win, // out ... converted string
                       path_len,        // in  ... size of path_to_pdf_win
                       path_to_pdf,     // in  ... input string
                       path_len - 1     // in  ... max wide chars to store
           ) != 0)
        {
            free(path_to_pdf_win);
            return ERR_IO;
        }

        if (_wfopen_s(&pdf_file, path_to_pdf_win, L"rb") != 0) {
            free(path_to_pdf_win);
            return ERR_IO;
        }

        free(path_to_pdf_win);
    #else
        if ((pdf_file = fopen(path_to_pdf, "rb")) == NULL)
            return ERR_IO;
    #endif

    sgl->pdf_data.deallocation_info |= DEALLOCATE_FILE;

    return sigil_set_pdf_file(sgl, pdf_file);
}

sigil_err_t sigil_set_pdf_buffer(sigil_t *sgl, char *pdf_content, size_t size)
{
    if (sgl == NULL || pdf_content == NULL || size <= 0)
        return ERR_PARAMETER;

    sgl->pdf_data.buffer = pdf_content;
    sgl->pdf_data.size = size;

    return ERR_NO;
}

sigil_err_t sigil_verify(sigil_t *sgl)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL)
        return ERR_PARAMETER;

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

void sigil_free(sigil_t **sgl)
{
    if (sgl == NULL || *sgl == NULL)
        return;

    if ((*sgl)->pdf_data.deallocation_info & DEALLOCATE_FILE) {
        fclose((*sgl)->pdf_data.file);
        (*sgl)->pdf_data.deallocation_info ^= DEALLOCATE_FILE;
    }
    if ((*sgl)->pdf_data.deallocation_info & DEALLOCATE_BUFFER) {
        free((*sgl)->pdf_data.buffer);
        (*sgl)->pdf_data.deallocation_info ^= DEALLOCATE_BUFFER;
    }

    if ((*sgl)->xref)
        xref_free((*sgl)->xref);

    free(*sgl);
    *sgl = NULL;
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

        sigil_free(&sgl);
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
