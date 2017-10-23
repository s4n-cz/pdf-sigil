#include "error.h"


void sigil_err_write(FILE *out, sigil_err_t err)
{
    if (err == ERR_NO) {
        fprintf(out, "NO ERROR\n");
        fflush(out);
        return;
    }

    if (err & ERR_ALLOC) {
        fprintf(out, "ERROR during allocation\n");
    }
    if (err & ERR_PARAM) {
        fprintf(out, "ERROR wrong parameter\n");
    }
    if (err & ERR_IO) {
        fprintf(out, "ERROR input/output\n");
    }
    //	...

    fflush(out);
}
