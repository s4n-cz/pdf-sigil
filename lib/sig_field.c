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
        if (err != ERR_NONE)
            return err;

        err = skip_word(sgl, "<<");
        if (err != ERR_NONE)
            return err;

        other_ft = 0;

        while (!other_ft && (err = parse_dict_key(sgl, &dict_key)) == ERR_NONE) {
            switch (dict_key) {
                case DICT_KEY_FT:
                    if (skip_word(sgl, "/Sig") == ERR_NONE) {
                        sgl->ref_sig_field = *(sgl->fields.entry[i]);
                        return ERR_NONE;
                    } else {
                        other_ft = 1;
                        break;
                    }
                case DICT_KEY_UNKNOWN:
                    err = skip_dict_unknown_value(sgl);
                    if (err != ERR_NONE)
                        return err;
                    break;
                default:
                    return ERR_PDF_CONTENT;
            }
        }
    }

    if (err != ERR_NONE)
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
    if (err != ERR_NONE)
        return err;

    err = skip_word(sgl, "<<");
    if (err != ERR_NONE)
        return err;

    while ((err = parse_dict_key(sgl, &dict_key)) == ERR_NONE) {
        switch (dict_key) {
            case DICT_KEY_FT:
                if (skip_word(sgl, "/Sig") != ERR_NONE)
                    return ERR_PDF_CONTENT;
                break;
            case DICT_KEY_V:
                if ((err = skip_leading_whitespaces(sgl)) != ERR_NONE)
                    return err;
                if ((err = pdf_peek_char(sgl, &c)) != ERR_NONE)
                    return err;
                if (c == '<') {
                    if ((err = get_curr_position(sgl, &offset)) != ERR_NONE)
                        return err;

                    sgl->offset_sig_dict = offset;

                    if ((err = skip_word(sgl, "<<")) != ERR_NONE)
                        return err;

                    err = skip_dictionary(sgl);
                    if (err != ERR_NONE)
                        return err;
                } else {
                    err = parse_indirect_reference(sgl, &(sgl->ref_sig_dict));
                    if (err != ERR_NONE)
                        return err;
                }
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

    return err;;
}

int sigil_sig_field_self_test(int verbosity)
{
    print_module_name("sig_field", verbosity);

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