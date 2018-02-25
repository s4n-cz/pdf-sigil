#include "auxiliary.h"
#include "sigil.h"


sigil_err_t process_trailer(sigil_t *sgl)
{
    // sigil_err_t err;

    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }


    // TODO

    return 0;
}

int sigil_trailer_self_test(int verbosity)
{
    print_module_name("trailer", verbosity);

    // TEST: fn determine_xref_type - STREAM
    print_test_item("xxx", verbosity);

    if (1)
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
