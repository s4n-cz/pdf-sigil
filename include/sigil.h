#ifndef PDF_SIGIL_SIGIL_H
#define PDF_SIGIL_SIGIL_H

#include <stdint.h>
#include "error.h"


#define MODE_VERIFY    0
#define MODE_SIGN      1

typedef struct {
    FILE *file;
	char *filename;
	uint32_t mode;
} sigil_t;

sigil_err_t sigil_init(sigil_t **sgl);

sigil_err_t sigil_config_file(sigil_t *sgl, const char *filename);
void sigil_config_mode(sigil_t *sgl, uint32_t mode);

sigil_err_t sigil_process(sigil_t *sgl);

// ...

void sigil_free(sigil_t *sgl);

#endif /* PDF_SIGIL_SIGIL_H */
