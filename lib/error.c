#include <stdio.h>
#include <string.h>
#include "error.h"


const char_t *sigil_err_string(sigil_err_t err)
{
    if (err == ERR_NO)
        return "NO ERROR";

    if (err & ERR_ALLOC)
        return "ERROR during allocation";

    if (err & ERR_PARAM)
        return "ERROR wrong parameter";

    if (err & ERR_IO)
        return "ERROR input/output";

    if (err & ERR_PDF_CONT)
        return "ERROR doesn't understand PDF file content";

    return "ERROR unknown";
}

int sigil_error_self_test(int quiet)
{
    if (!quiet)
        printf("\n + Testing module: error\n");

    // TEST: error codes
    if (!quiet)
        printf("    - %-30s", "error codes");

    if ((ERR_NO    +    ERR_ALLOC     +    ERR_PARAM +
         ERR_IO    +    ERR_PDF_CONT  +    ERR_5     +
         ERR_6     +    ERR_7         +    ERR_8     +
         ERR_9     +    ERR_10        +    ERR_11    +
         ERR_12    +    ERR_13        +    ERR_14    +
         ERR_15    +    ERR_16
        ) != 0xffff || (ERR_NO != 0)                 )
    {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: fn sigil_err_string
    if (!quiet)
        printf("    - %-30s", "fn sigil_err_string");

    if (strcmp(sigil_err_string(ERR_NO   ), "NO ERROR"               ) != 0 ||
        strcmp(sigil_err_string(ERR_ALLOC), "ERROR during allocation") != 0 ||
        strcmp(sigil_err_string(0x800000 ), "ERROR unknown"          ) != 0 )
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
    if (!quiet) {
        printf("   FAILED\n");
        fflush(stdout);
    }

    return 1;
}
