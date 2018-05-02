/** @file
 *
 */

#ifndef PDF_SIGIL_CONTENTS_H
#define PDF_SIGIL_CONTENTS_H

#include "types.h"


sigil_err_t parse_contents(sigil_t *sgl);

int sigil_contents_self_test(int verbosity);

#endif //PDF_SIGIL_CONTENTS_H
