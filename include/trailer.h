#ifndef PDF_SIGIL_TRAILER_H
#define PDF_SIGIL_TRAILER_H

#include "error.h"


sigil_err_t process_trailer(sigil_t *sgl);

int sigil_trailer_self_test(int verbosity);

#endif /* PDF_SIGIL_TRAILER_H */
