#include <openssl/evp.h>
#include <openssl/x509.h>
#include <types.h>
#include <string.h>
#include "auxiliary.h"
#include "config.h"
#include "constants.h"
#include "cryptography.h"
#include "types.h"


sigil_err_t hex_to_dec(const char *in, size_t in_len, unsigned char *out, size_t *out_len)
{
    int first,
        second;

    if (in == NULL || (in_len % 2) != 0 || out == NULL || out_len == NULL)
        return ERR_PARAMETER;

    *out_len = 0;

    for (size_t i = 0; i < in_len; i += 2) {
        if (is_digit(in[i])) {
            first = (in[i] - '0') << 4;
        } else if (in[i] >= 'A' && in[i] <= 'F') {
            first = (in[i] - 55) << 4;
        } else if (in[i] >= 'a' && in[i] <= 'f') {
            first = (in[i] - 87) << 4;
        } else {
            return ERR_PDF_CONTENT;
        }

        if (is_digit(in[i + 1])) {
            second = in[i + 1] - '0';
        } else if (in[i + 1] >= 'A' && in[i + 1] <= 'F') {
            second = in[i + 1] - 55;
        } else if (in[i + 1] >= 'a' && in[i + 1] <= 'f') {
            second = in[i + 1] - 87;
        } else {
            return ERR_PDF_CONTENT;
        }

        out[*out_len] = (unsigned char)(first + second);

        (*out_len)++;
    }

    out[*out_len] = '\0';

    return ERR_NO;
}

void print_computed_hash(sigil_t *sgl)
{
    if (sgl == NULL || sgl->computed_hash_len <= 0)
        return;

    printf("\nCOMPUTED HASH: ");

    for (int i = 0; i < sgl->computed_hash_len; i++) {
        printf("%02x ", sgl->computed_hash[i]);
    }

    printf("\n");
}

sigil_err_t compute_sha1_hash_over_range(sigil_t *sgl)
{
    sigil_err_t err;
    EVP_MD_CTX *ctx;
    range_t *range;
    size_t bytes_left;
    char *current_data;
    size_t current_length;
    size_t read_size;

    if (sgl == NULL || sgl->byte_range == NULL)
        return ERR_PARAMETER;

    current_data = malloc(sizeof(*current_data) * (HASH_UPDATE_SIZE + 1));
    if (current_data == NULL)
        return ERR_ALLOCATION;

    sigil_zeroize(current_data, sizeof(*current_data) * (HASH_UPDATE_SIZE + 1));

    // initialize context
    if ((ctx = EVP_MD_CTX_create()) == NULL)
        return ERR_ALLOCATION;

    if(EVP_DigestInit_ex(ctx, EVP_sha1(), NULL) != 1) {
        err = ERR_OPENSSL;
        goto failed;
    }

    range = sgl->byte_range;

    while (range != NULL) {
        err = pdf_move_pos_abs(sgl, range->start);
        if (err != ERR_NO)
            return err;

        bytes_left = range->length;

        while (bytes_left > 0) {
            current_length = MIN(HASH_UPDATE_SIZE, bytes_left);

            err = pdf_read(sgl, current_length, current_data, &read_size);
            if (err != ERR_NO)
                return err;
            if (current_length != read_size)
                return ERR_IO;

            if (EVP_DigestUpdate(ctx, current_data, current_length) != 1) {
                err = ERR_OPENSSL;
                goto failed;
            }

            bytes_left -= current_length;
        }

        range = range->next;
    }

    // process last pieces of data from context
    if (EVP_DigestFinal_ex(ctx, sgl->computed_hash, &(sgl->computed_hash_len)) != 1) {
        err = ERR_OPENSSL;
        goto failed;
    }

    free(current_data);

    EVP_MD_CTX_destroy(ctx);

    return ERR_NO;

failed:
    if (ctx != NULL)
        EVP_MD_CTX_destroy(ctx);

    free(current_data);

    return err;
}

sigil_err_t load_certificates(sigil_t *sgl)
{
    sigil_err_t err;
    cert_t *certificate;
    unsigned char *tmp_cert;
    const unsigned char *const_tmp;
    size_t cert_length;
    size_t tmp_cert_len;

    if (sgl == NULL)
        return ERR_PARAMETER;

    certificate = sgl->certificates;

    while (certificate != NULL) {
        if (certificate->x509 != NULL) {
            X509_free(certificate->x509);
            certificate->x509 = NULL;
        }

        cert_length = strlen(certificate->cert_hex);

        tmp_cert = malloc(sizeof(*(certificate->cert_hex)) * ((cert_length + 1) / 2 + 1));
        if (tmp_cert == NULL)
            return ERR_ALLOCATION;

        sigil_zeroize(tmp_cert,
                      sizeof(*(certificate->cert_hex)) * ((cert_length + 1) / 2 + 1));

        err = hex_to_dec(certificate->cert_hex, cert_length, tmp_cert, &tmp_cert_len);
        if (err != ERR_NO)
            return err;

        const_tmp = tmp_cert;

        certificate->x509 = d2i_X509(NULL, &const_tmp, tmp_cert_len);
        if (certificate->x509 == NULL) {
            free(tmp_cert);
            return ERR_OPENSSL;
        }

        free(tmp_cert);

        certificate = certificate->next;
    }

    return ERR_NO;
}

sigil_err_t verify_signing_certificate(sigil_t *sgl)
{
    X509_STORE_CTX *ctx;
    cert_t *additional_cert;
    STACK_OF(X509) *trusted_chain;

    if (sgl == NULL || sgl->certificates == NULL)
        return ERR_PARAMETER;

    trusted_chain = sk_X509_new_null();

    additional_cert = sgl->certificates->next;

    while (additional_cert != NULL) {
        if (sk_X509_push(trusted_chain, additional_cert->x509) == 0) {
            sk_X509_free(trusted_chain);
            return ERR_OPENSSL;
        }

        additional_cert = additional_cert->next;
    }

    ctx = X509_STORE_CTX_new();
    if (ctx == NULL) {
        sk_X509_free(trusted_chain);
        return ERR_OPENSSL;
    }

    // initialize store context
    if (X509_STORE_CTX_init(ctx, sgl->trusted_store, sgl->certificates->x509, trusted_chain) != 1) {
        sk_X509_free(trusted_chain);
        return ERR_OPENSSL;
    }


    // signing certificate to be verified
    X509_STORE_CTX_set_cert(ctx, sgl->certificates->x509);

    // verify
    if (X509_verify_cert(ctx) == 1) {
        // verification successful
        sgl->signing_cert_status = CERT_STATUS_VERIFIED;
    } else {
        // verification not successful
        sgl->signing_cert_status = CERT_STATUS_FAILED;
    }

    sk_X509_free(trusted_chain);

    X509_STORE_CTX_free(ctx);

    return ERR_NO;
}
