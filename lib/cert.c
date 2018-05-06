#include <stdlib.h>
#include <types.h>
#include <string.h>
#include "auxiliary.h"
#include "cert.h"
#include "config.h"
#include "constants.h"
#include "types.h"
#include "sigil.h"


static sigil_err_t parse_one_cert(sigil_t *sgl, cert_t **result)
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

    if ((err = skip_word(sgl, "<")) != ERR_NONE)
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
        if ((err = pdf_get_char(sgl, &c)) != ERR_NONE)
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
            return ERR_NONE;
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

    if ((err = skip_leading_whitespaces(sgl)) != ERR_NONE)
        return err;

    if ((err = pdf_peek_char(sgl, &c)) != ERR_NONE)
        return err;

    if (c == '[') // multiple certs
        additional_certs = 1;

    if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
        return err;

    // read signing certificate
    err = parse_one_cert(sgl, &(sgl->certificates));
    if (err != ERR_NONE)
        return err;

    if (!additional_certs)
        return ERR_NONE;

    next_cert = &(sgl->certificates);

    // read other following certs for verifying authenticity of the signing one
    while (1) {
        next_cert = &((*next_cert)->next);

        if ((err = skip_leading_whitespaces(sgl)) != ERR_NONE)
            return err;

        if ((err = pdf_peek_char(sgl, &c)) != ERR_NONE)
            return err;

        if (c == ']') {
            if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
                return err;

            return ERR_NONE;
        }

        err = parse_one_cert(sgl, next_cert);
        if (err != ERR_NONE)
            return err;
    }
}

void cert_free(cert_t *cert)
{
    if (cert == NULL)
        return;

    cert_free(cert->next);

    if (cert->cert_hex != NULL) {
        sigil_zeroize(cert->cert_hex,
                      sizeof(*cert->cert_hex) * strlen(cert->cert_hex));
        free(cert->cert_hex);
    }

    if (cert->x509 != NULL)
        X509_free(cert->x509);

    sigil_zeroize(cert, sizeof(*cert));
    free(cert);
}

int sigil_cert_self_test(int verbosity)
{
    print_module_name("cert", verbosity);

    // place for possible later tests
    // ...

    // all tests done
    print_module_result(1, verbosity);
    return 0;

/*
failed:
    print_test_result(0, verbosity);
    print_module_result(0, verbosity);

    return 1;
*/
}