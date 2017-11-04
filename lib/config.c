#include <stdio.h>
#include "config.h"

int sigil_config_self_test(int quiet)
{
    if (!quiet)
        printf("\n + Validating config values\n");

    // TEST: HEADER_SEARCH_OFFSET
    if (!quiet)
        printf("    - %-30s", "HEADER_SEARCH_OFFSET");

    if (HEADER_SEARCH_OFFSET < 0) {
        if (!quiet)
            printf("FAILED\n");

        goto failed;
    }

    if (!quiet)
        printf("OK\n");

    // TEST: XREF_SEARCH_OFFSET
    if (!quiet)
        printf("    - %-30s", "XREF_SEARCH_OFFSET");

    if (XREF_SEARCH_OFFSET < 20) {
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
