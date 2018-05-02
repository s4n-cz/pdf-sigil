#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "auxiliary.h"
#include "cert.h"
#include "contents.h"
#include "constants.h"
#include "sig_dict.h"
#include "types.h"

#define SUBFILTER_MAX    30


static sigil_err_t parse_subfilter(sigil_t *sgl)
{
    sigil_err_t err;
    int count = 0;
    char tmp[SUBFILTER_MAX],
            c;

    if (sgl == NULL)
        return ERR_PARAMETER;

    sigil_zeroize(tmp, SUBFILTER_MAX * sizeof(*tmp));

    err = parse_word(sgl, "/");
    if (err != ERR_NONE)
        return err;

    while ((err = pdf_peek_char(sgl, &c)) == ERR_NONE) {
        if (is_whitespace(c)) {
            if (count <= 0)
                return ERR_PDF_CONTENT;
            break;
        } else {
            if (count >= SUBFILTER_MAX - 1)
                return ERR_PDF_CONTENT;
            tmp[count++] = c;
        }

        if ((err = pdf_move_pos_rel(sgl, 1)) != ERR_NONE)
            return err;
    }

    if (err != ERR_NONE)
        return err;

    if (strncmp(tmp, "adbe.x509.rsa_sha1", 18) == 0) {
        sgl->subfilter_type = SUBFILTER_adbe_x509_rsa_sha1;
    } else {
        sgl->subfilter_type = SUBFILTER_UNKNOWN;
    }

    return ERR_NONE;
}

static sigil_err_t parse_byte_range(sigil_t *sgl)
{
    sigil_err_t err;
    range_t **byte_range;
    size_t start,
           length;

    if (sgl == NULL)
        return ERR_PARAMETER;

    err = parse_word(sgl, "[");
    if (err != ERR_NONE)
        return err;

    byte_range = &(sgl->byte_range);

    while (1) {
        if (parse_word(sgl, "]") == ERR_NONE)
            return ERR_NONE;

        err = parse_number(sgl, &start);
        if (err != ERR_NONE)
            return err;

        err = parse_number(sgl, &length);
        if (err != ERR_NONE)
            return err;

        if (start + length > sgl->pdf_data.size)
            return ERR_PDF_CONTENT;

        if (*byte_range == NULL) {
            *byte_range = malloc(sizeof(**byte_range));
            if (*byte_range == NULL)
                return ERR_ALLOCATION;
            sigil_zeroize(*byte_range, sizeof(**byte_range));
        }

        (*byte_range)->start = start;
        (*byte_range)->length = length;

        byte_range = &((*byte_range)->next);
    }
}

sigil_err_t process_sig_dict(sigil_t *sgl)
{
    sigil_err_t err;
    dict_key_t dict_key;

    if (sgl == NULL)
        return ERR_PARAMETER;

    if (sgl->offset_sig_dict <= 0 && sgl->ref_sig_dict.object_num > 0) {
        err = pdf_goto_obj(sgl, &(sgl->ref_sig_dict));
        if (err != ERR_NONE)
            return err;
    } else {
        err = pdf_move_pos_abs(sgl, sgl->offset_sig_dict);
        if (err != ERR_NONE)
            return err;
    }

    err = parse_word(sgl, "<<");
    if (err != ERR_NONE)
        return err;

    while ((err = parse_dict_key(sgl, &dict_key)) == ERR_NONE) {
        switch (dict_key) {
            case DICT_KEY_SubFilter:
                if ((err = parse_subfilter(sgl)) != ERR_NONE)
                    return err;

                break;
            case DICT_KEY_Cert:
                err = parse_certs(sgl);
                if (err != ERR_NONE)
                    return err;

                break;
            case DICT_KEY_Contents:
                err = parse_contents(sgl);
                if (err != ERR_NONE)
                    return err;

                break;
            case DICT_KEY_ByteRange:
                if ((err = parse_byte_range(sgl)) != ERR_NONE)
                    return err;

                break;
            case DICT_KEY_UNKNOWN:
                err = skip_dict_unknown_value(sgl);
                if (err != ERR_NONE)
                    return err;

                break;
            default:
                return ERR_PDF_CONTENT;
        }
    }

    if (err == ERR_END_OF_DICT)
        return ERR_NONE;

    return err;

}
