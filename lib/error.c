#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auxiliary.h"
#include "error.h"


void sigil_err_write(FILE *out, sigil_err_t err)
{
    if (err == ERR_NO) {
        fprintf(out, "NO ERROR\n");
        fflush(out);
        return;
    }

    if (err & ERR_ALLOC) {
        fprintf(out, "ERROR during allocation\n");
    }
    if (err & ERR_PARAM) {
        fprintf(out, "ERROR wrong parameter\n");
    }
    if (err & ERR_IO) {
        fprintf(out, "ERROR input/output\n");
    }
    if (err & ERR_PDF_CONT) {
        fprintf(out, "ERROR doesn't understand PDF file content\n");
    }
    //	...

    fflush(out);
}

int sigil_error_self_test(int quiet)
{
    if (!quiet)
        printf("\n\n + Testing module: error\n");

    // TEST: error codes
    if (!quiet)
        printf("    - %-30s", "error codes");

    if ((ERR_NO    +    ERR_ALLOC     +    ERR_PARAM +
         ERR_IO    +    ERR_PDF_CONT  +    ERR_5     +
         ERR_6     +    ERR_7         +    ERR_8     +
         ERR_9     +    ERR_10        +    ERR_11    +
         ERR_12    +    ERR_13        +    ERR_14    +
         ERR_15    +    ERR_16
        ) != 0xffff || (ERR_NO != 0))
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: sigil_err_write(...)
    if (!quiet)
        printf("    - %-30s", "fn sigil_err_write");

    char *file_buf;
    size_t len;
    FILE *filestream = open_memstream(&file_buf, &len);

    if (filestream) {
        sigil_err_write(filestream, ERR_NO);
        sigil_err_write(filestream, ERR_ALLOC | ERR_IO);
        sigil_err_write(filestream, 0xffff);
        fflush(filestream);

        const char *expected_data =                      \
            "NO ERROR\n"                                 \
            "ERROR during allocation\n"                  \
            "ERROR input/output\n"                       \
            "ERROR during allocation\n"                  \
            "ERROR wrong parameter\n"                    \
            "ERROR input/output\n"                       \
            "ERROR doesn't understand PDF file content\n";

        int cmp = strncmp(file_buf, expected_data, len);

        fclose(filestream);
        free(file_buf);

        if (cmp == 0) {
            if (!quiet)
                printf("OK\n");
        } else {
            if (!quiet)
                printf("FAILED\n");

            goto failed;
        }
    } else {
        if (!quiet)
            printf("...\n");
    }

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
