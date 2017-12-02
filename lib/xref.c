#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "config.h"
#include "error.h"
#include "sigil.h"
#include "xref.h"


sigil_err_t read_startxref(sigil_t *sgl)
{
    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    // jump to end of file
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

    if (sgl->startxref == 0) {
        return (sigil_err_t)ERR_PDF_CONT;
    }

    return (sigil_err_t)ERR_NO;
}

int sigil_xref_self_test(int verbosity)
{
    v_print("\n + Testing module: xref\n", 0, verbosity, 1);

    // prepare
    sigil_t *sgl = NULL;

    if (sigil_init(&sgl) != ERR_NO) {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    // TEST: fn read_startxref
    v_print("    - fn read_startxref", -35, verbosity, 2);

    char_t *correct_1 = "startxref\n"                                            \
                        "1234567890\n"                                           \
                        "\045\045EOF"; // %%EOF
    sgl->file = fmemopen(correct_1,
                         (strlen(correct_1) + 1) * sizeof(*correct_1),
                         "r");
    if (sgl->file == NULL) {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    if (read_startxref(sgl) != ERR_NO ||
        sgl->file_size != 27          ||
        sgl->startxref != 1234567890  )
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // all tests done
    v_print(COLOR_GREEN "   PASSED\n" COLOR_RESET, 0, verbosity, 1);
    return 0;

failed:
    if (sgl->file) {
        fclose(sgl->file);
    }

    v_print(COLOR_RED "   FAILED\n" COLOR_RESET, 0, verbosity, 1);
    return 1;

}
