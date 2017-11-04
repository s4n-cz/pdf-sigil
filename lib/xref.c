#include <stdlib.h>
#include <string.h>
#include "auxiliary.h"
#include "config.h"
#include "error.h"
#include "sigil.h"
#include "xref.h"


sigil_err_t read_startxref(sigil_t *sgl)
{
    // function parameter checks
    if (sgl == NULL || sgl->file == NULL) {
        return (sigil_err_t)ERR_PARAM;
    }

    // jump to end of file
    if (fseek(sgl->file, 0, SEEK_END) != 0) {
        return (sigil_err_t)ERR_IO;
    }

    // get file size
    size_t file_size = ftell(sgl->file);
    if (file_size < 0) {
        return (sigil_err_t)ERR_IO;
    }

    // jump max XREF_SEARCH_OFFSET bytes from end
    size_t jump_pos = MAX(0, file_size - XREF_SEARCH_OFFSET);
    if (fseek(sgl->file, jump_pos, SEEK_SET) != 0) {
        return (sigil_err_t)ERR_IO;
    }

    // prepare buffer for data
    size_t buf_len = (file_size - jump_pos + 1) + 1;
    char *buf = malloc(buf_len * sizeof(char));
    if (buf == NULL) {
        return (sigil_err_t)ERR_ALLOC;
    }

    // copy data from the end of file
    size_t read = fread(buf, sizeof(char), buf_len - 1, sgl->file);
    if (read <= 0) {
        free(buf);
        return (sigil_err_t)ERR_IO;
    }
    buf[read] = '\0';

    // search for 'startxref' from the end
    for (int i = read - 9; i >= 0; i--) {
        if (memcmp(buf + i, "startxref", 9) == 0) {
            i += 9;
            while (i < read && is_whitespace(buf[i])) {
                i++;
            }
            if (!is_digit(buf[i])) {
                free(buf);
                return (sigil_err_t)ERR_PDF_CONT;
            }
            while (i < read && is_digit(buf[i])) {
                sgl->startxref = 10 * sgl->startxref + buf[i] - '0';
            }
            break;
        }
    }

    free(buf);

    if (sgl->startxref == 0) {
        return (sigil_err_t)ERR_PDF_CONT;
    }
    return (sigil_err_t)ERR_NO;
}
