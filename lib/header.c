#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "config.h"
#include "constants.h"
#include "header.h"
#include "sigil.h"

sigil_err_t process_header(sigil_t *sgl)
{
    sigil_err_t err;
    size_t offset;
    char tmp[6],
         c;
    size_t read_size;
    size_t pdf_x, pdf_y;

    if (sgl == NULL)
        return ERR_PARAMETER;

    for (offset = 0; offset < HEADER_SEARCH_OFFSET; offset++) {
        err = pdf_move_pos_abs(sgl, offset);
        if (err != ERR_NONE)
            return err;

        err = pdf_read(sgl, 5, tmp, &read_size);
        if (err != ERR_NONE)
            return err;
        if (read_size != 5)
            return ERR_PDF_CONTENT;

        if (strncmp(tmp, "\x25PDF-", 5) != 0)
            continue;

        if ((err = parse_number(sgl, &pdf_x)) != ERR_NONE)
            return err;

        if ((err = pdf_get_char(sgl, &c)) != ERR_NONE)
            return err;
        if (c != '.')
            return ERR_PDF_CONTENT;

        if ((err = parse_number(sgl, &pdf_y)) != ERR_NONE)
            return err;

        if ((pdf_x == 1 && pdf_y >= 0 && pdf_y <= 7) ||
            (pdf_x == 2 && pdf_y == 0))
        {
            sgl->pdf_x = (short)pdf_x;
            sgl->pdf_y = (short)pdf_y;
        } else {
            return ERR_PDF_CONTENT;
        }

        sgl->offset_pdf_start = offset;

        return ERR_NONE;
    }

    return ERR_PDF_CONTENT;
}

int sigil_header_self_test(int verbosity)
{
    sigil_t *sgl = NULL;
    char c;

    print_module_name("header", verbosity);

    // TEST: fn process_header
    print_test_item("fn process_header", verbosity);

    {
        char *sstream_1 = "\x25PDF-1.1 x";
        if ((sgl = test_prepare_sgl_content(sstream_1, strlen(sstream_1) + 1)) == NULL)
            goto failed;

        if (process_header(sgl) != ERR_NONE ||
            sgl->pdf_x != 1               ||
            sgl->pdf_y != 1               ||
            sgl->offset_pdf_start != 0)
        {
            goto failed;
        }

        if (skip_leading_whitespaces(sgl) != ERR_NONE)
            goto failed;
        if ((pdf_get_char(sgl, &c)) != ERR_NONE || c != 'x')
            goto failed;

        sigil_free(&sgl);

        char *sstream_2 = "\x1a\x5e\x93\x7e\x6f\x3c\x6a\x71\xbf\xda\x54\x91\xe5"\
                          "\x86\x08\x84\xaf\x8e\x89\x44\xab\xc4\x58\x0c\xb9\x31"\
                          "\xd3\x8c\x0f\xc0\x43\x1a\xa5\x07\x4f\xe2\x98\xb3\xd8"\
                          "\x53\x4b\x5d\x4b\xd6\x48\x26\x98\x09\xde\x0d"        \
                          "\x25PDF-1.2 x";
        if ((sgl = test_prepare_sgl_content(sstream_2, strlen(sstream_2) + 1)) == NULL)
            goto failed;

        if (process_header(sgl) != ERR_NONE ||
            sgl->pdf_x != 1               ||
            sgl->pdf_y != 2               ||
            sgl->offset_pdf_start != 50)
        {
            goto failed;
        }

        if (skip_leading_whitespaces(sgl) != ERR_NONE)
            goto failed;
        if ((pdf_get_char(sgl, &c)) != ERR_NONE || c != 'x')
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
