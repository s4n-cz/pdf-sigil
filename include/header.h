#ifndef PDF_SIGIL_HEADER_H
#define PDF_SIGIL_HEADER_H

#include "sigil.h"

#ifndef SIGIL_ERR_T
#define SIGIL_ERR_T
    typedef uint32_t sigil_err_t;
#endif /* SIGIL_ERR_T */


sigil_err_t process_header(sigil_t *sgl);

int sigil_header_self_test(int verbosity);

#endif /* PDF_SIGIL_HEADER_H */
