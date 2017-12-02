#include <stdio.h>
#include "auxiliary.h"
#include "config.h"

int sigil_config_self_test(int verbosity)
{
    v_print("\n + Validating config values\n", 0, verbosity, 1);

    // TEST: HEADER_SEARCH_OFFSET
    v_print("    - HEADER_SEARCH_OFFSET", -35, verbosity, 2);

    if (HEADER_SEARCH_OFFSET < 0) {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // TEST: XREF_SEARCH_OFFSET
    v_print("    - XREF_SEARCH_OFFSET", -35, verbosity, 2);

    if (XREF_SEARCH_OFFSET < 20) {
        v_print(COLOR_RED "FAILED\n" COLOR_RESET, 0, verbosity, 2);
        goto failed;
    }

    v_print(COLOR_GREEN "OK\n" COLOR_RESET, 0, verbosity, 2);

    // all tests done
    v_print(COLOR_GREEN "   PASSED\n" COLOR_RESET, 0, verbosity, 1);
    return 0;

failed:
    v_print(COLOR_RED "   FAILED\n"COLOR_RESET, 0, verbosity, 1);
    return 1;
}
