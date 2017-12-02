#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
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

int sigil_error_self_test(int verbosity)
{
    v_print("\n + Testing module: error\n", 0, verbosity, 1);

    // TEST: error codes
    v_print("    - error codes", -35, verbosity, 2);

    if ((ERR_NO    +    ERR_ALLOC     +    ERR_PARAM +
         ERR_IO    +    ERR_PDF_CONT  +    ERR_5     +
         ERR_6     +    ERR_7         +    ERR_8     +
         ERR_9     +    ERR_10        +    ERR_11    +
         ERR_12    +    ERR_13        +    ERR_14    +
         ERR_15    +    ERR_16
        ) != 0xffff || ERR_NO != 0)
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // TEST: fn sigil_err_string
    v_print("    - fn sigil_err_string", -35, verbosity, 2);

    if (strcmp(sigil_err_string(ERR_NO   ), "NO ERROR"               ) != 0 ||
        strcmp(sigil_err_string(ERR_ALLOC), "ERROR during allocation") != 0 ||
        strcmp(sigil_err_string(0x800000 ), "ERROR unknown"          ) != 0 )
    {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // all tests done
    v_print(COLOR_GREEN "   PASSED\n" COLOR_RESET, 0, verbosity, 1);
    return 0;

failed:
    v_print(COLOR_RED "   FAILED\n" COLOR_RESET, 0, verbosity, 1);
    return 1;
}
