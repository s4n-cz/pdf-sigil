#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "config.h"
#include "error.h"
#include "sigil.h"
#include "xref.h"


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

sigil_err_t read_startxref(sigil_t *sgl)
{
    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    // jump to the end of file
    if (fseek(sgl->file, 0, SEEK_END) != 0) {
        return (sigil_err_t)ERR_IO;
    }

    // get file size
    if (sgl->file_size <= 0) {
        sgl->file_size = ftell(sgl->file);
        if (sgl->file_size < 0) {
            return (sigil_err_t)ERR_IO;
        }
    }

    // jump max XREF_SEARCH_OFFSET bytes from end
    size_t jump_pos = MAX(0, (ssize_t)sgl->file_size - XREF_SEARCH_OFFSET);
    if (fseek(sgl->file, jump_pos, SEEK_SET) != 0) {
        return (sigil_err_t)ERR_IO;
    }

    // prepare buffer for data
    size_t buf_len = sgl->file_size - jump_pos + 1;
    char_t *buf = malloc(buf_len * sizeof(char_t));
    if (buf == NULL) {
        return (sigil_err_t)ERR_ALLOC;
    }

    // copy data from the end of file
    size_t read = fread(buf, sizeof(*buf), buf_len - 1, sgl->file);
    if (read <= 0) {
        free(buf);
        return (sigil_err_t)ERR_IO;
    }
    buf[read] = '\0';

    // search for 'startxref' from the end
    for (int i = read - 9; i >= 0; i--) {
        if (memcmp(buf + i, "startxref", 9) == 0) {
            i += 9;
            while (i < read && is_whitespace(buf[i])) {
                i++;
            }
            if (!is_digit(buf[i])) {
                free(buf);
                return (sigil_err_t)ERR_PDF_CONT;
            }
            sgl->startxref = 0;
            while (i < read && is_digit(buf[i])) {
                sgl->startxref = 10 * sgl->startxref + buf[i] - '0';
                i++;
            }
            break;
        }
    }

    free(buf);

    if (sgl->startxref == 0)
        return (sigil_err_t)ERR_PDF_CONT;

    return (sigil_err_t)ERR_NO;
}

sigil_err_t process_xref(sigil_t *sgl)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL || sgl->file == NULL || sgl->startxref == 0) {
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

int sigil_xref_self_test(int verbosity)
{
    print_module_name("xref", verbosity);

    // prepare
    sigil_t *sgl = NULL;

    if (sigil_init(&sgl) != ERR_NO) {
        verbosity = MIN(verbosity, 1); // do not print test result "FAILED"
        goto failed;
    }

    // TEST: fn read_startxref
    print_test_item("fn read_startxref", verbosity);

    char_t *correct_1 = "startxref\n"                                            \
                        "1234567890\n"                                           \
                        "\045\045EOF"; // %%EOF
    sgl->file = fmemopen(correct_1,
                         (strlen(correct_1) + 1) * sizeof(*correct_1),
                         "r");
    if (sgl->file == NULL) {
        goto failed;
    }

    if (read_startxref(sgl) != ERR_NO ||
        sgl->file_size != 27          ||
        sgl->startxref != 1234567890  )
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    fclose(sgl->file);

    // TEST: fn determine_xref_type - TABLE
    print_test_item("fn determine_xref_type 1", verbosity);

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
    if (sgl)
        sigil_free(sgl);

    print_test_result(0, verbosity);
    print_module_result(0, verbosity);
    return 1;
}
