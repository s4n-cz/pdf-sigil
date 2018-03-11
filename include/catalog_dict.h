#ifndef PDF_SIGIL_CATALOG_DICT_H
#define PDF_SIGIL_CATALOG_DICT_H

#include "types.h"


sigil_err_t process_catalog_dictionary(sigil_t *sgl);

int sigil_header_self_test(int verbosity);

#endif /* PDF_SIGIL_CATALOG_DICT_H */
