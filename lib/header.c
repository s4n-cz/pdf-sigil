#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

    const char_t expected[] = {'%', 'P', 'D', 'F', '-'};
    size_t offset = 0;
    int found = 0,
        c;

    while ((c = fgetc(sgl->file)) != EOF && found < 8 &&
            offset - found <= HEADER_SEARCH_OFFSET    )
    {
        // count offset from start to '%' character
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
    sgl->pdf_start_offset = offset - found;

    return (sigil_err_t)ERR_NO;
}

int sigil_header_self_test(int verbosity)
{
    v_print("\n + Testing module: header\n", 0, verbosity, 1);

    // prepare
    sigil_t *sgl = NULL;

    if (sigil_init(&sgl) != ERR_NO)
        goto failed;

    // TEST: correct_1
    v_print("    - correct_1", -35, verbosity, 2);

    char_t *correct_1 = "\x25PDF-1.1\n"             \
                        "abcdefghijklmnopqrstuvwxyz";
    sgl->file = fmemopen(correct_1,
                         (strlen(correct_1) + 1) * sizeof(*correct_1),
                         "r");
    if (sgl->file == NULL) {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    if (process_header(sgl) != ERR_NO ||
        sgl->pdf_x != 1               ||
        sgl->pdf_y != 1               ||
        sgl->pdf_start_offset != 0     )
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    fclose(sgl->file);
    sgl->file = NULL;

    // TEST: correct_2
    v_print("    - correct_2", -35, verbosity, 2);

    char_t *correct_2 = "\x1a\x5e\x93\x7e\x6f\x3c\x6a\x71\xbf\xda\x54\x91\xe5" \
                        "\x86\x08\x84\xaf\x8e\x89\x44\xab\xc4\x58\x0c\xb9\x31" \
                        "\xd3\x8c\x0f\xc0\x43\x1a\xa5\x07\x4f\xe2\x98\xb3\xd8" \
                        "\x53\x4b\x5d\x4b\xd6\x48\x26\x98\x09\xde\x0d"         \
                        "\x25PDF-1.2"                                          \
                        "\x55\xa1\x77\xd3\x47\xab\xc6\x87\xf3\xbc\x2d\x8a\x9f" \
                        "\x0e\x47\xbb\x74\xd2\x71\x28\x94\x53\x92\xae\x2b\x17" \
                        "\xd0\x6a\x9c\x13\x84\xc1\x07\x44\xc0\x81\xb8\xd6\x9c" \
                        "\x31\x08\x13\xd4\xc2\xd6\x2d\xaf\xfb\xea\x6f";
    sgl->file = fmemopen(correct_2,
                         (strlen(correct_2) + 1) * sizeof(*correct_2),
                         "r");
    if (sgl->file == NULL) {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    if (process_header(sgl) != ERR_NO ||
        sgl->pdf_x != 1               ||
        sgl->pdf_y != 2               ||
        sgl->pdf_start_offset != 50    )
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    fclose(sgl->file);
    sgl->file = NULL;

    // TEST: wrong_1
    v_print("    - wrong_1", -35, verbosity, 2);

    char_t *wrong_1 = "\x25\x25PPD\x25PDF-\x25PDF-1\x25PDF-1..@PDF-1.3";
    sgl->file = fmemopen(wrong_1,
                         (strlen(wrong_1) + 1) * sizeof(*wrong_1),
                         "r");
    if (sgl->file == NULL) {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    if (process_header(sgl) == ERR_NO) {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    fclose(sgl->file);
    sgl->file = NULL;

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
