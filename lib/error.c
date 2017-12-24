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
        return "ERROR corrupted PDF file";

    return "ERROR unknown";
}

int sigil_error_self_test(int verbosity)
{
    print_module_name("error", verbosity);

    // TEST: error codes
    print_test_item("error codes", verbosity);

    if ((ERR_NO    +    ERR_ALLOC     +    ERR_PARAM +
         ERR_IO    +    ERR_PDF_CONT  +    ERR_5     +
         ERR_6     +    ERR_7         +    ERR_8     +
         ERR_9     +    ERR_10        +    ERR_11    +
         ERR_12    +    ERR_13        +    ERR_14    +
         ERR_15    +    ERR_16
        ) != 0xffff || ERR_NO != 0)
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    // TEST: fn sigil_err_string
    print_test_item("fn sigil_err_string", verbosity);

    if (strcmp(sigil_err_string(ERR_NO   ), "NO ERROR"               ) != 0 ||
        strcmp(sigil_err_string(ERR_ALLOC), "ERROR during allocation") != 0 ||
        strcmp(sigil_err_string(0x800000 ), "ERROR unknown"          ) != 0 )
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    print_test_result(0, verbosity);
    print_module_result(0, verbosity);
    return 1;
}
