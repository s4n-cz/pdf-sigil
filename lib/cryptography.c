#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <types.h>
#include <string.h>
#include <sigil.h>
#include "auxiliary.h"
#include "config.h"
#include "constants.h"
#include "cryptography.h"
#include "types.h"


/** @brief Converts hexadecimal characters to the decimal value. Operates with
 *         characters both on input and output, 2 chars on the input produces
 *         1 char on the output
 *
 * @param in input - hexadecimal characters
 * @param in_len length of the input
 * @param out output buffer
 * @param out_len length of the output written (without the terminating null)
 * @return ERR_NONE if success
 */
static sigil_err_t hex_to_dec(const char *in, size_t in_len, unsigned char *out, size_t *out_len)
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

    return ERR_NONE;
}

sigil_err_t compute_digest_pkcs1(sigil_t *sgl)
{
    sigil_err_t err;
    char *update_data = NULL;
    EVP_MD_CTX *ctx = NULL;
    const EVP_MD *evp_md;
    const ASN1_OBJECT *md_obj = NULL;
    range_t *range;
    size_t bytes_left;
    size_t current_length;
    size_t read_size;
    unsigned char tmp_hash[EVP_MAX_MD_SIZE];
    unsigned int tmp_hash_len;

    if (sgl == NULL || sgl->byte_range == NULL)
        return ERR_PARAMETER;

    update_data = malloc(sizeof(*update_data) * (HASH_UPDATE_SIZE + 1));
    if (update_data == NULL) {
        err = ERR_ALLOCATION;
        goto end;
    }

    sigil_zeroize(update_data, sizeof(*update_data) * (HASH_UPDATE_SIZE + 1));

    // initialize digest context
    if ((ctx = EVP_MD_CTX_create()) == NULL)
        return ERR_ALLOCATION;

    X509_ALGOR_get0(&md_obj, NULL, NULL, sgl->digest_algorithm);
    evp_md = EVP_get_digestbyobj(md_obj);
    if (evp_md == NULL) {
        err = ERR_OPENSSL;
        goto end;
    }

    // only allowed algorithms
    switch (EVP_MD_type(evp_md)) {
        case NID_sha1:
            sgl->hash_fn = HASH_FN_sha1;
            break;
        case NID_sha256:
            sgl->hash_fn = HASH_FN_sha256;
            break;
        case NID_sha384:
            sgl->hash_fn = HASH_FN_sha384;
            break;
        case NID_sha512:
            sgl->hash_fn = HASH_FN_sha512;
            break;
        case NID_ripemd160:
            sgl->hash_fn = HASH_FN_ripemd160;
            break;
        default:
            err = ERR_DIGEST_TYPE;
            goto end;
    }

    if (EVP_DigestInit_ex(ctx, evp_md, NULL) != 1) {
        err = ERR_OPENSSL;
        goto end;
    }

    range = sgl->byte_range;

    while (range != NULL) {
        err = pdf_move_pos_abs(sgl, range->start);
        if (err != ERR_NONE)
            return err;

        bytes_left = range->length;

        while (bytes_left > 0) {
            current_length = MIN(HASH_UPDATE_SIZE, bytes_left);

            err = pdf_read(sgl, current_length, update_data, &read_size);
            if (err != ERR_NONE)
                return err;
            if (current_length != read_size)
                return ERR_IO;

            if (EVP_DigestUpdate(ctx, update_data, current_length) != 1) {
                err = ERR_OPENSSL;
                goto end;
            }

            bytes_left -= current_length;
        }

        range = range->next;
    }

    // process last pieces of data from context
    if (EVP_DigestFinal_ex(ctx, tmp_hash, &tmp_hash_len) != 1) {
        err = ERR_OPENSSL;
        goto end;
    }

    sgl->digest_computed = ASN1_OCTET_STRING_new();
    if (ASN1_OCTET_STRING_set(sgl->digest_computed, tmp_hash, tmp_hash_len) == 0) {
        err = ERR_OPENSSL;
        goto end;
    }

    err = ERR_NONE;

end:
    if (update_data != NULL)
        free(update_data);
    if (ctx != NULL)
        EVP_MD_CTX_destroy(ctx);

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
        if (err != ERR_NONE)
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

    return ERR_NONE;
}

sigil_err_t load_digest(sigil_t *sgl)
{
    sigil_err_t              err;
    char                    *contents;
    size_t                   contents_len;
    unsigned char           *tmp_contents = NULL;
    size_t                   tmp_contents_len;
    const unsigned char     *const_tmp;
    ASN1_OCTET_STRING       *oc_str = NULL;
    EVP_PKEY                *pub_key = NULL;
    RSA                     *rsa = NULL;
    unsigned char           *rsa_out_str = NULL;
    int                      rsa_out_len;
    const unsigned char     *const_rsa_out;
    X509_SIG                *sig = NULL;
    const X509_SIG          *const_sig = NULL;
    const X509_ALGOR        *tmp_alg = NULL;
    const ASN1_OCTET_STRING *tmp_hash = NULL;

    if (sgl == NULL || sgl->contents == NULL || sgl->certificates == NULL)
        return ERR_PARAMETER;

    contents = sgl->contents->contents_hex;
    contents_len = strlen(contents);

    tmp_contents = malloc(sizeof(*contents) * ((contents_len + 1) / 2 + 1));
    if (tmp_contents == NULL) {
        err = ERR_ALLOCATION;
        goto end;
    }

    sigil_zeroize(tmp_contents, sizeof(*contents) * ((contents_len + 1) / 2 + 1));

    err = hex_to_dec(contents, contents_len, tmp_contents, &tmp_contents_len);
    if (err != ERR_NONE)
        goto end;

    const_tmp = tmp_contents;

    oc_str = d2i_ASN1_OCTET_STRING(NULL, &const_tmp, tmp_contents_len);
    if (oc_str == NULL) {
        err = ERR_OPENSSL;
        goto end;
    }

    pub_key = X509_get_pubkey(sgl->certificates->x509);
    if (pub_key == NULL) {
        err = ERR_OPENSSL;
        goto end;
    }

    rsa = EVP_PKEY_get1_RSA(pub_key);
    if (rsa == NULL) {
        err = ERR_OPENSSL;
        goto end;
    }

    rsa_out_len = RSA_size(rsa) - 11;
    if (rsa_out_len <= 0) {
        err = ERR_OPENSSL;
        goto end;
    }

    rsa_out_str = malloc(sizeof(*rsa_out_str) * rsa_out_len);
    if (rsa_out_str == NULL) {
        err = ERR_OPENSSL;
        goto end;
    }

    sigil_zeroize(rsa_out_str, sizeof(*rsa_out_str) * rsa_out_len);

    rsa_out_len = RSA_public_decrypt(ASN1_STRING_length(oc_str),
                                     oc_str->data,
                                     rsa_out_str,
                                     rsa,
                                     RSA_PKCS1_PADDING);

    const_rsa_out = rsa_out_str;

    sig = d2i_X509_SIG(NULL, &const_rsa_out, rsa_out_len);
    if (sig == NULL) {
        err = ERR_OPENSSL;
        goto end;
    }

    const_sig = sig;

    X509_SIG_get0(const_sig, &tmp_alg, &tmp_hash);

    sgl->digest_algorithm = X509_ALGOR_dup((X509_ALGOR *)tmp_alg);
    sgl->digest_original = ASN1_OCTET_STRING_dup(tmp_hash);

    err = ERR_NONE;

end:
    if (tmp_contents != NULL)
        free(tmp_contents);
    if (oc_str != NULL)
        ASN1_OCTET_STRING_free(oc_str);
    if (pub_key != NULL)
        EVP_PKEY_free(pub_key);
    if (rsa != NULL)
        RSA_free(rsa);
    if (rsa_out_str != NULL)
        free(rsa_out_str);
    if (sig != NULL)
        X509_SIG_free(sig);

    return err;
}

/* How to add support for CRL checking:
 *
 * X509_VERIFY_PARAM *param;
 *
 * param = X509_VERIFY_PARAM_new();
 * if (param == NULL)
 *     return ERR_OPENSSL;
 *
 * // set verify flags -> X509_V_FLAG_CRL_CHECK or X509_V_FLAG_CRL_CHECK_ALL
 * X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_CRL_CHECK_ALL);
 *
 * // after X509_STORE_CTX_init(...)
 * X509_VERIFY_PARAM_set1(ctx->param, param);
 *
 * // before call to X509_verify_cert(...) add the CRLs
 *
 * // at the end
 * X509_VERIFY_PARAM_free(param);
*/
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
        sgl->result_cert_verification = CERT_STATUS_VERIFIED;
    } else {
        // verification not successful
        sgl->result_cert_verification = CERT_STATUS_FAILED;
    }

    sk_X509_free(trusted_chain);

    X509_STORE_CTX_free(ctx);

    return ERR_NONE;
}

sigil_err_t compare_digest(sigil_t *sgl)
{
    if (sgl == NULL)
        return ERR_PARAMETER;

    sgl->result_digest_comparison = HASH_CMP_RESULT_DIFFER;

    if (sgl->digest_original == NULL || sgl->digest_computed == NULL)
        return ERR_PARAMETER;

    if (ASN1_STRING_cmp(sgl->digest_original, sgl->digest_computed) == 0)
        sgl->result_digest_comparison = HASH_CMP_RESULT_MATCH;

    return ERR_NONE;
}

int sigil_cryptography_self_test(int verbosity)
{
    sigil_t *sgl;
    sigil_err_t err;

    print_module_name("cryptography", verbosity);

    // TEST: fn compare_digest
    print_test_item("fn compare_digest", verbosity);

    {
        const unsigned char *str_1 = (const unsigned char *)"123456789abcdef";
        const unsigned char *str_2 = (const unsigned char *)"fedcba987654321";
        int result;

        err = sigil_init(&sgl);
        if (err != ERR_NONE || sgl == NULL)
            goto failed;

        sgl->digest_original = ASN1_OCTET_STRING_new();
        if (sgl->digest_original == NULL)
            goto failed;

        sgl->digest_computed = ASN1_OCTET_STRING_new();
        if (sgl->digest_computed == NULL)
            goto failed;

        ASN1_OCTET_STRING_set(sgl->digest_original, str_1, -1);
        ASN1_OCTET_STRING_set(sgl->digest_computed, str_1, -1);

        if (compare_digest(sgl) != ERR_NONE)
            goto failed;

        if (sigil_get_data_integrity_result(sgl, &result) != ERR_NONE)
            goto failed;

        if (result != HASH_CMP_RESULT_MATCH)
            goto failed;

        ASN1_OCTET_STRING_free(sgl->digest_computed);

        sgl->digest_computed = ASN1_OCTET_STRING_new();
        if (sgl->digest_computed == NULL)
            goto failed;

        ASN1_OCTET_STRING_set(sgl->digest_computed, str_2, -1);

        if (compare_digest(sgl) != ERR_NONE)
            goto failed;

        if (sigil_get_data_integrity_result(sgl, &result) != ERR_NONE)
            goto failed;

        if (result != HASH_CMP_RESULT_DIFFER)
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    if (sgl != NULL)
        sigil_free(&sgl);

    print_test_result(0, verbosity);
    print_module_result(0, verbosity);

    return 1;
}