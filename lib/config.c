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

    // TEST: REF_ARRAY_PREALLOCATION
    print_test_item("REF_ARRAY_PREALLOCATION", verbosity);

    if (REF_ARRAY_PREALLOCATION < 1)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: CERT_HEX_PREALLOCATION
    print_test_item("CERT_HEX_PREALLOCATION", verbosity);

    if (CERT_HEX_PREALLOCATION < 1)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: CONTENTS_PREALLOCATION
    print_test_item("CONTENTS_PREALLOCATION", verbosity);

    if (CONTENTS_PREALLOCATION < 1)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: THRESHOLD_FILE_BUFFERING
    print_test_item("THRESHOLD_FILE_BUFFERING", verbosity);

    if (THRESHOLD_FILE_BUFFERING < 0)
        goto failed;

    print_test_result(1, verbosity);

    // TEST: MAX_FILE_UPDATES
    print_test_item("MAX_FILE_UPDATES", verbosity);

    if (MAX_FILE_UPDATES < 1)
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
