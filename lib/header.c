#include <stdint.h>
#include "auxiliary.h"
#include "config.h"
#include "error.h"
#include "sigil.h"

sigil_err_t process_header(sigil_t *sgl)
{
    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    const char expected[] = {'%', 'P', 'D', 'F', '-'};
    size_t offset = 0;
    int found = 0,
        c;

    while ((c = fgetc(sgl->file)) != EOF && found < 8 &&
            offset - found <= HEADER_SEARCH_OFFSET    )
    {
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
            if (is_digit(c)) {
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
            if (is_digit(c)) {
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

    // offset counted with header -> subtract header size
    sgl->pdf_start_offset = offset - 8;

    return (sigil_err_t)ERR_NO;
}

int sigil_header_self_test(int quiet)
{
    if (!quiet)
        printf("\n + Testing module: header\n");

    // prepare
    sigil_t *sgl = NULL;

    if (sigil_init(&sgl) != ERR_NO)
        goto failed;

    // TEST: correct_1
    sgl->file = fopen("test/test_header/correct_1", "r");
    if (sgl->file == NULL)
        goto failed;

    if (!quiet)
        printf("    - %-30s", "correct_1");

    if (process_header(sgl) != ERR_NO ||
        sgl->pdf_x != 1                     ||
        sgl->pdf_y != 1                     )
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    fclose(sgl->file);

    // TEST: correct_2
    sgl->file = fopen("test/test_header/correct_2", "r");
    if (sgl->file == NULL)
        goto failed;

    if (!quiet)
        printf("    - %-30s", "correct_2");

    if (process_header(sgl) != ERR_NO ||
        sgl->pdf_x != 1                     ||
        sgl->pdf_y != 2                     )
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    fclose(sgl->file);

    // TODO add at least one wrong

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
