#include <stdio.h>
#include <string.h>
#include "auxiliary.h"
#include "config.h"
#include "error.h"
#include "header.h"
#include "sigil.h"
#include "xref.h"

int main(int argc, char **argv)
{
    int quiet  = 0,
        failed = 0;

    if (argc == 2 && (strcmp(argv[1], "-q"     ) == 0  ||
                      strcmp(argv[1], "--quiet") == 0  ))
    {
        quiet = 1;
    }

    if (!quiet)
        printf("\n STARTING TEST PROCEDURE\n");

    // call self_test function for each module
    if (sigil_config_self_test(quiet) != 0)
        failed++;
    if (sigil_auxiliary_self_test(quiet) != 0)
        failed++;
    if (sigil_error_self_test(quiet) != 0)
        failed++;
    if (sigil_header_self_test(quiet) != 0)
        failed++;
    if (sigil_xref_self_test(quiet) != 0)
        failed++;
    if (sigil_sigil_self_test(quiet) != 0)
        failed++;

    if (!quiet)
        printf("\n TOTAL FAILED: %d\n", failed);

    return (failed != 0);
}
