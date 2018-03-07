#include "auxiliary.h"
#include "constants.h"
#include "trailer.h"

sigil_err_t process_trailer(sigil_t *sgl)
{
    sigil_err_t err;
    keyword_t keyword;
    dict_key_t dict_key;
    char c;

    if (sgl == NULL)
        return ERR_PARAMETER;

    // read "trailer"
    err = parse_keyword(sgl, &keyword);
    if (err != ERR_NO)
        return err;
    if (keyword != KEYWORD_trailer)
        return ERR_PDF_CONTENT;

    err = skip_leading_whitespaces(sgl);
    if (err != ERR_NO)
        return err;
    if ((pdf_get_char(sgl, &c)) != ERR_NO || c != '<')
        return ERR_PDF_CONTENT;
    if ((pdf_get_char(sgl, &c)) != ERR_NO || c != '<')
        return ERR_PDF_CONTENT;

    while ((err = parse_dict_key(sgl, &dict_key)) == ERR_NO) {
        switch (dict_key) {
            case DICT_KEY_Size:
                err = parse_number(sgl, &(sgl->xref->size_from_trailer));
                if (err != ERR_NO)
                    return err;
                break;
            case DICT_KEY_Prev:
                err = parse_number(sgl, &(sgl->xref->prev_section));
                if (err != ERR_NO)
                    return err;
                break;
            case DICT_KEY_Root:
                err = parse_indirect_reference(sgl, &(sgl->ref_catalog_dict));
                if (err != ERR_NO)
                    return err;
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

    return err;
}

int sigil_trailer_self_test(int verbosity)
{
    print_module_name("trailer", verbosity);

    // TEST: fn determine_xref_type - STREAM
    print_test_item("fn process_trailer", verbosity);

    if (1)
    {
        goto failed;
    }

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);

    return 0;

failed:
    print_test_result(0, verbosity);
    print_module_result(0, verbosity);

    return 1;
}
