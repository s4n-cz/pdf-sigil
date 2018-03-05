#ifndef PDF_SIGIL_SIGIL_H
#define PDF_SIGIL_SIGIL_H

#include "types.h"


sigil_err_t sigil_init(sigil_t **sgl);

sigil_err_t sigil_set_pdf_file(sigil_t *sgl, FILE *pdf_file);
sigil_err_t sigil_set_pdf_path(sigil_t *sgl, const char *path_to_pdf);
sigil_err_t sigil_set_pdf_buffer(sigil_t *sgl, char *pdf_content, size_t size);

sigil_err_t sigil_verify(sigil_t *sgl);

// ... get functions TODO

void sigil_free(sigil_t **sgl);

int sigil_sigil_self_test(int verbosity);

#endif /* PDF_SIGIL_SIGIL_H */
