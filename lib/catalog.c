#include "acroform.h"
#include "auxiliary.h"
#include "catalog.h"
#include "constants.h"
#include "types.h"

sigil_err_t process_catalog(sigil_t *sgl)
{
    sigil_err_t err;
    size_t offset;
    dict_key_t dict_key;
    char c;

    if (sgl == NULL)
        return ERR_PARAMETER;

    if (sgl->ref_catalog_dict.object_num <= 0 &&
        sgl->ref_catalog_dict.generation_num <= 0)
    {
        return ERR_NO_DATA;
    }

    err = pdf_goto_obj(sgl, &(sgl->ref_catalog_dict));
    if (err != ERR_NONE)
        return err;

    err = skip_word(sgl, "<<");
    if (err != ERR_NONE)
        return err;

    while ((err = parse_dict_key(sgl, &dict_key)) == ERR_NONE) {
        switch (dict_key) {
            case DICT_KEY_AcroForm:
                if ((err = skip_leading_whitespaces(sgl)) != ERR_NONE)
                    return err;

                if ((err = pdf_peek_char(sgl, &c)) != ERR_NONE)
                    return err;

                if (c == '<') {
                    if ((err = get_curr_position(sgl, &offset)) != ERR_NONE)
                        return err;

                    sgl->offset_acroform = offset;

                    if ((err = skip_word(sgl, "<<")) != ERR_NONE)
                        return err;

                    err = skip_dictionary(sgl);
                    if (err != ERR_NONE)
                        return err;
                } else {
                    err = parse_indirect_reference(sgl, &(sgl->ref_acroform));
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

    return err;
}

int sigil_catalog_self_test(int verbosity)
{
    print_module_name("catalog", verbosity);

    // place for possible later tests
    // ...

    // all tests done
    print_module_result(1, verbosity);
    return 0;

failed:
    print_test_result(0, verbosity);
    print_module_result(0, verbosity);

    return 1;
}