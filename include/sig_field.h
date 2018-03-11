#ifndef PDF_SIGIL_SIG_FIELD_H
#define PDF_SIGIL_SIG_FIELD_H

#include "types.h"


sigil_err_t find_sig_field(sigil_t *sgl);

sigil_err_t process_sig_field(sigil_t *sgl);

int sigil_sigil_self_test(int verbosity);

#endif /* PDF_SIGIL_SIG_FIELD_H */
