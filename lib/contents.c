#include <stdlib.h>
#include <types.h>
#include "auxiliary.h"
#include "config.h"
#include "constants.h"
#include "contents.h"
#include "types.h"
#include "sigil.h"


sigil_err_t parse_contents(sigil_t *sgl)
{
    sigil_err_t err;
    char **data;
    char c;
    size_t position;

    if (sgl == NULL)
        return ERR_PARAMETER;

    if ((err = skip_leading_whitespaces(sgl)) != ERR_NONE)
        return err;

    if (sgl->contents != NULL)
        contents_free(sgl);

    if ((err = skip_word(sgl, "<")) != ERR_NONE)
        return err;

    sgl->contents = malloc(sizeof(*(sgl->contents)));
    if (sgl->contents == NULL)
        return ERR_ALLOCATION;

    sigil_zeroize(sgl->contents, sizeof(*(sgl->contents)));

    data = &(sgl->contents->contents_hex);

    *data = malloc(sizeof(**data) * CONTENTS_PREALLOCATION);
    if (*data == NULL)
        return ERR_ALLOCATION;

    sigil_zeroize(*data, sizeof(**data) * CONTENTS_PREALLOCATION);

    sgl->contents->capacity = CONTENTS_PREALLOCATION;

    position = 0;

    while (1) {
        if ((err = pdf_get_char(sgl, &c)) != ERR_NONE)
            return err;

        // not enough space, allocate double
        if (position >= sgl->contents->capacity) {
            *data = realloc(*data, sizeof(**data) * sgl->contents->capacity * 2);
            if (*data == NULL)
                return ERR_ALLOCATION;

            sigil_zeroize(*data + sgl->contents->capacity,
                          sizeof(**data) * sgl->contents->capacity);

            sgl->contents->capacity *= 2;
        }

        if (c == '>') {
            (*data)[position] = '\0';
            return ERR_NONE;
        }

        (*data)[position] = c;

        position++;
    }
}