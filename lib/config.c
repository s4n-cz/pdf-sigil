#include "auxiliary.h"
#include "config.h"

int sigil_config_self_test(int verbosity)
{
    print_module_name("config", verbosity);

    // TEST: HEADER_SEARCH_OFFSET
    print_test_item("HEADER_SEARCH_OFFSET", verbosity);

    if (HEADER_SEARCH_OFFSET < 0)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: XREF_SEARCH_OFFSET
    print_test_item("XREF_SEARCH_OFFSET", verbosity);

    if (XREF_SEARCH_OFFSET < 20)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: XREF_PREALLOCATION
    print_test_item("XREF_PREALLOCATION", verbosity);

    if (XREF_PREALLOCATION < 1)
        goto failed;

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    print_test_result(0, verbosity);
    print_module_result(0, verbosity);
    return 1;
}
