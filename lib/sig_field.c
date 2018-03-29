#include <types.h>
#include "auxiliary.h"
#include "constants.h"
#include "sig_field.h"


sigil_err_t find_sig_field(sigil_t *sgl)
{
    sigil_err_t err;
    dict_key_t dict_key;
    int other_ft;

    if (sgl == NULL)
        return ERR_PARAMETER;

    err = ERR_NO_DATA;

    for (size_t i = 0; i < sgl->fields.capacity; i++) {
        if (sgl->fields.entry[i] == NULL)
            continue;

        err = pdf_goto_obj(sgl, sgl->fields.entry[i]);
        if (err != ERR_NO)
            return err;

        err = parse_word(sgl, "<<");
        if (err != ERR_NO)
            return err;

        other_ft = 0;

        while (!other_ft && (err = parse_dict_key(sgl, &dict_key)) == ERR_NO) {
            switch (dict_key) {
                case DICT_KEY_FT:
                    if (parse_word(sgl, "/Sig") == ERR_NO) {
                        sgl->ref_sig_field = *(sgl->fields.entry[i]);
                        return ERR_NO;
                    } else {
                        other_ft = 1;
                        break;
                    }
                case DICT_KEY_UNKNOWN:
                    err = skip_dict_unknown_value(sgl);
                    if (err != ERR_NO)
                        return err;
                    break;
                default:
                    return ERR_PDF_CONTENT;
            }
        }
    }

    if (err != ERR_NO)
        return err;

    return ERR_NO_DATA;
}

sigil_err_t process_sig_field(sigil_t *sgl)
{
    sigil_err_t err;
    size_t offset;
    dict_key_t dict_key;
    char c;

    if (sgl == NULL)
        return ERR_PARAMETER;

    err = pdf_goto_obj(sgl, &(sgl->ref_sig_field));
    if (err != ERR_NO)
        return err;

    err = parse_word(sgl, "<<");
    if (err != ERR_NO)
        return err;

    while ((err = parse_dict_key(sgl, &dict_key)) == ERR_NO) {
        switch (dict_key) {
            case DICT_KEY_FT:
                if (parse_word(sgl, "/Sig") != ERR_NO)
                    return ERR_PDF_CONTENT;
                break;
            case DICT_KEY_V:
                if ((err = skip_leading_whitespaces(sgl)) != ERR_NO)
                    return err;
                if ((err = pdf_peek_char(sgl, &c)) != ERR_NO)
                    return err;
                if (c == '<') {
                    if ((err = get_curr_position(sgl, &offset)) != ERR_NO)
                        return err;

                    sgl->offset_sig_dict = offset;

                    if ((err = parse_word(sgl, "<<")) != ERR_NO)
                        return err;

                    err = skip_dictionary(sgl);
                    if (err != ERR_NO)
                        return err;
                } else {
                    err = parse_indirect_reference(sgl, &(sgl->ref_sig_dict));
                    if (err != ERR_NO)
                        return err;
                }
                break;
            case DICT_KEY_UNKNOWN:
                err = skip_dict_unknown_value(sgl);
                if (err != ERR_NO)
                    return err;
                break;
            default:
                return ERR_PDF_CONTENT;
        }
    }

    if (err == ERR_END_OF_DICT)
        return ERR_NO;

    return err;;
}