#include <string.h>
#include "auxiliary.h"
#include "error.h"
#include "sigil.h"
#include "xref.h"


sigil_err_t process_trailer(sigil_t *sgl)
{
    sigil_err_t err;
    keyword_t keyword;
    dict_key_t dict_key;

    // function parameter checks
    if (sgl == NULL || sgl->file == NULL)
        return (sigil_err_t)ERR_PARAM;

    // read "trailer"
    err = parse_keyword(sgl->file, &keyword);
    if (err != ERR_NO)
        return err;
    if (keyword != KEYWORD_trailer)
        return (sigil_err_t)ERR_PDF_CONT;

    err = skip_leading_whitespaces(sgl->file);
    if (err != ERR_NO)
        return err;
    // if merged into one if statement with the '&&' operator in between, it's
    //   optimized out and position in the file is not changed
    if (fgetc(sgl->file) != '<')
        return 1;
    if (fgetc(sgl->file) != '<')
        return 1;

    while ((err = parse_dict_key(sgl->file, &dict_key)) == ERR_NO) {
        switch (dict_key) {
            case DICT_KEY_Size:
                err = parse_number(sgl->file, &sgl->xref->size_from_trailer);
                if (err != ERR_NO)
                    return err;
                break;
            case DICT_KEY_Prev:
                err = parse_number(sgl->file, &sgl->xref->prev_section);
                if (err != ERR_NO)
                    return err;
                break;
            case DICT_KEY_Root:
                err = parse_indirect_reference(sgl->file, &sgl->ref_catalog_dict);
                if (err != ERR_NO)
                    return err;
                break;
            case DICT_KEY_unknown:
                err = skip_dict_unknown_value(sgl->file);
                if (err != ERR_NO)
                    return err;
                break;
            default:
                return (sigil_err_t)ERR_PDF_CONT;
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
