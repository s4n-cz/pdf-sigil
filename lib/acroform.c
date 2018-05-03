#include <types.h>
#include "acroform.h"
#include "auxiliary.h"
#include "constants.h"
#include "types.h"


sigil_err_t process_acroform(sigil_t *sgl)
{
    sigil_err_t err;
    dict_key_t dict_key;

    if (sgl == NULL)
        return ERR_PARAMETER;

    if (sgl->offset_acroform <= 0 && sgl->ref_acroform.object_num > 0) {
        err = pdf_goto_obj(sgl, &(sgl->ref_acroform));
        if (err != ERR_NONE)
            return err;
    } else {
        err = pdf_move_pos_abs(sgl, sgl->offset_acroform);
        if (err != ERR_NONE)
            return err;
    }

    err = skip_word(sgl, "<<");
    if (err != ERR_NONE)
        return err;

    while ((err = parse_dict_key(sgl, &dict_key)) == ERR_NONE) {
        switch (dict_key) {
            case DICT_KEY_Fields:
                err = parse_ref_array(sgl, &(sgl->fields));
                if (err != ERR_NONE)
                    return err;
                break;
            case DICT_KEY_SigFlags:
                err = parse_number(sgl, &(sgl->sig_flags));
                if (err != ERR_NONE)
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

int sigil_acroform_self_test(int verbosity)
{
    print_module_name("acroform", verbosity);

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
