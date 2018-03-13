#include <openssl/evp.h>
#include <types.h>
#include "auxiliary.h"
#include "config.h"
#include "constants.h"
#include "cryptography.h"
#include "types.h"


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

    EVP_MD_CTX_destroy(ctx);

    return ERR_NO;

failed:
    if (ctx != NULL)
        EVP_MD_CTX_destroy(ctx);

    return err;
}