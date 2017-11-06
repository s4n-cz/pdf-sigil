#include <stdlib.h>
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
    char *buf = malloc(buf_len * sizeof(char));
    if (buf == NULL) {
        return (sigil_err_t)ERR_ALLOC;
    }

    // copy data from the end of file
    size_t read = fread(buf, sizeof(char), buf_len - 1, sgl->file);
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

int sigil_xref_self_test(int quiet)
{
    if (!quiet)
        printf("\n + Testing module: xref\n");

    // prepare
    sigil_t *sgl = NULL;

    if (sigil_init(&sgl) != ERR_NO)
        goto failed;

    // TEST: fn read_startxref
    if (!quiet)
        printf("    - %-30s", "fn read_startxref");

    char *correct_1 = "startxref\n"                                            \
                      "1234567890\n"                                           \
                      "%%EOF";
    sgl->file = fmemopen(correct_1,
                         (strlen(correct_1) + 1) * sizeof(char),
                         "r");
    if (sgl->file == NULL)
        goto failed;

    if (read_startxref(sgl) != ERR_NO ||
        sgl->file_size != 27          ||
        sgl->startxref != 1234567890  )
    {
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
    if (sgl->file) {
        fclose(sgl->file);
    }

    if (!quiet) {
        printf("   FAILED\n");
        fflush(stdout);
    }

    return 1;

}
