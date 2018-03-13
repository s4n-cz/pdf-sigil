#include <stdlib.h>
#include <types.h>
#include "auxiliary.h"
#include "cert.h"
#include "config.h"
#include "constants.h"
#include "types.h"
#include "sigil.h"


sigil_err_t parse_one_cert(sigil_t *sgl, cert_t **result)
{
    sigil_err_t err;
    char c;
    char **data;
    size_t position;

    if (sgl == NULL || result == NULL)
        return ERR_PARAMETER;

    if (*result != NULL) {
        cert_free(*result);
        *result = NULL;
    }

    if ((err = parse_word(sgl, "<")) != ERR_NO)
        return err;

    *result = malloc(sizeof(**result));
    if (*result == NULL)
        return ERR_ALLOCATION;

    sigil_zeroize(*result, sizeof(**result));

    data = &((*result)->cert_hex);

    *data = malloc(sizeof(*(*result)->cert_hex)
                                 * CERT_HEX_PREALLOCATION);
    if (*data == NULL)
        return ERR_ALLOCATION;

    sigil_zeroize(*data, sizeof(**data) * CERT_HEX_PREALLOCATION);

    (*result)->capacity = CERT_HEX_PREALLOCATION;

    position = 0;

    while (1) {
        if ((err = pdf_get_char(sgl, &c)) != ERR_NO)
            return err;

        // not enough space, allocate double
        if (position >= (*result)->capacity) {
            *data = realloc(*data, sizeof(**data) * (*result)->capacity * 2);
            if (*data == NULL)
                return ERR_ALLOCATION;

            sigil_zeroize(*data + (*result)->capacity,
                          sizeof(**data) * (*result)->capacity);

            (*result)->capacity *= 2;
        }

        if (c == '>') {
            (*data)[position] = '\0';
            return ERR_NO;
        }

        (*data)[position] = c;

        position++;
    }
}

sigil_err_t parse_certs(sigil_t *sgl)
{
    sigil_err_t err;
    int additional_certs;
    cert_t **next_cert;
    char c;

    if (sgl == NULL)
        return ERR_PARAMETER;

    additional_certs = 0;

    if ((err = skip_leading_whitespaces(sgl)) != ERR_NO)
        return err;

    if ((err = pdf_peek_char(sgl, &c)) != ERR_NO)
        return err;

    if (c == '[') // multiple certs
        additional_certs = 1;

    if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NO)
        return err;

    // read signing certificate
    err = parse_one_cert(sgl, &(sgl->certificates));
    if (err != ERR_NO)
        return err;

    if (!additional_certs)
        return ERR_NO;

    next_cert = &(sgl->certificates);

    // read other following certs for verifying authenticity of the signing one
    while (1) {
        next_cert = &((*next_cert)->next);

        if ((err = skip_leading_whitespaces(sgl)) != ERR_NO)
            return err;

        if ((err = pdf_peek_char(sgl, &c)) != ERR_NO)
            return err;

        if (c == ']') {
            if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NO)
                return err;

            return ERR_NO;
        }

        err = parse_one_cert(sgl, next_cert);
        if (err != ERR_NO)
            return err;
    }
}