#include <stdio.h>
#include "auxiliary.h"
#include "constants.h"
#include "trailer.h"

sigil_err_t process_trailer(sigil_t *sgl)
{
    sigil_err_t err;
    dict_key_t dict_key;

    if (sgl == NULL)
        return ERR_PARAMETER;

    err = skip_word(sgl, "trailer");
    if (err != ERR_NONE)
        return err;

    err = skip_word(sgl, "<<");
    if (err != ERR_NONE)
        return err;

    while ((err = parse_dict_key(sgl, &dict_key)) == ERR_NONE) {
        switch (dict_key) {
            case DICT_KEY_Size:
                if (sgl->xref->size_from_trailer > 0) {
                    err = skip_dict_unknown_value(sgl);
                } else {
                    err = parse_number(sgl, &(sgl->xref->size_from_trailer));
                }
                if (err != ERR_NONE)
                    return err;
                break;
            case DICT_KEY_Prev:
                err = parse_number(sgl, &(sgl->xref->prev_section));
                if (err != ERR_NONE)
                    return err;
                break;
            case DICT_KEY_Root:
                if (sgl->ref_catalog_dict.object_num > 0 ||
                    sgl->ref_catalog_dict.generation_num > 0)
                {
                    err = skip_dict_unknown_value(sgl);
                } else {
                    err = parse_indirect_reference(sgl, &(sgl->ref_catalog_dict));
                }
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

int sigil_trailer_self_test(int verbosity)
{
    print_module_name("trailer", verbosity);

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