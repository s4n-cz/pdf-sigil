#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "acroform.h"
#include "auxiliary.h"
#include "catalog_dict.h"
#include "config.h"
#include "constants.h"
#include "cryptography.h"
#include "header.h"
#include "sig_dict.h"
#include "sig_field.h"
#include "sigil.h"
#include "trailer.h"
#include "types.h"
#include "xref.h"

sigil_err_t sigil_init(sigil_t **sgl)
{
    // function parameter checks
    if (sgl == NULL)
        return ERR_PARAMETER;

    *sgl = malloc(sizeof(sigil_t));
    if (*sgl == NULL)
        return ERR_ALLOCATION;

    sigil_zeroize(*sgl, sizeof(*sgl));

    // set default values
    (*sgl)->pdf_data.file                   = NULL;
    (*sgl)->pdf_data.buffer                 = NULL;
    (*sgl)->pdf_data.buf_pos                = 0;
    (*sgl)->pdf_data.size                   = 0;
    (*sgl)->pdf_data.deallocation_info      = 0;
    (*sgl)->pdf_x                           = 0;
    (*sgl)->pdf_y                           = 0;
    (*sgl)->sig_flags                       = 0;
    (*sgl)->subfilter_type                  = SUBFILTER_UNKNOWN;
    (*sgl)->xref_type                       = XREF_TYPE_UNSET;
    (*sgl)->ref_acroform.object_num         = 0;
    (*sgl)->ref_acroform.generation_num     = 0;
    (*sgl)->ref_catalog_dict.object_num     = 0;
    (*sgl)->ref_catalog_dict.generation_num = 0;
    (*sgl)->ref_sig_dict.object_num         = 0;
    (*sgl)->ref_sig_dict.generation_num     = 0;
    (*sgl)->ref_sig_field.object_num        = 0;
    (*sgl)->ref_sig_field.generation_num    = 0;
    (*sgl)->offset_acroform                 = 0;
    (*sgl)->offset_pdf_start                = 0;
    (*sgl)->offset_sig_dict                 = 0;
    (*sgl)->offset_startxref                = 0;
    (*sgl)->digest_algorithm                = NULL;
    (*sgl)->digest_computed                 = NULL;
    (*sgl)->digest_original                 = NULL;
    (*sgl)->fields.capacity                 = 0;
    (*sgl)->fields.entry                    = NULL;
    (*sgl)->byte_range                      = NULL;
    (*sgl)->certificates                    = NULL;
    (*sgl)->contents                        = NULL;
    (*sgl)->xref                            = NULL;
    (*sgl)->trusted_store                   = X509_STORE_new();
    (*sgl)->result_cert_verification        = CERT_STATUS_UNKNOWN;
    (*sgl)->result_digest_comparison        = HASH_CMP_RESULT_UNKNOWN;

    return ERR_NONE;
}

sigil_err_t sigil_set_pdf_file(sigil_t *sgl, FILE *pdf_file)
{
    size_t processed,
           total_processed;
    char *content = NULL;

    if (sgl == NULL || pdf_file == NULL)
        return ERR_PARAMETER;

    if (sgl->pdf_data.file != NULL && sgl->pdf_data.file != pdf_file)
        fclose(sgl->pdf_data.file);

    sgl->pdf_data.file = pdf_file;

    // get file size
    // - 1) jump to the end of file
    if (fseek(sgl->pdf_data.file, 0, SEEK_END) != 0)
        return ERR_IO;

    // - 2) read current position
    sgl->pdf_data.size = (size_t)ftell(sgl->pdf_data.file);
    if (sgl->pdf_data.size < 0)
        return ERR_IO;

    // - 3) jump back to the beginning
    if (fseek(sgl->pdf_data.file, 0, SEEK_SET) != 0)
        return ERR_IO;

    if (sgl->pdf_data.size < THRESHOLD_FILE_BUFFERING) {
        content = malloc(sizeof(char) * (sgl->pdf_data.size + 1));
        if (content == NULL) {
            // fallback to using the file
            return ERR_NONE;
        }

        total_processed = 0;

        while (total_processed * sizeof(char) < sgl->pdf_data.size) {
            processed = fread(content + total_processed, sizeof(char),
                              sgl->pdf_data.size, sgl->pdf_data.file);
            total_processed += processed;
            if (processed <= 0 ||
                total_processed * sizeof(char) > sgl->pdf_data.size)
            {
                // fallback to using the file
                free(content);
                return ERR_NONE;
            }
        }

        if (total_processed * sizeof(char) != sgl->pdf_data.size) {
            // fallback to using the file
            free(content);
            return ERR_NONE;
        }

        content[total_processed] = '\0';

        sgl->pdf_data.buffer = content;
        sgl->pdf_data.deallocation_info |= DEALLOCATE_BUFFER;
    }

    return ERR_NONE;
}

sigil_err_t sigil_set_pdf_path(sigil_t *sgl, const char *path_to_pdf)
{
    FILE *pdf_file = NULL;

    if (sgl == NULL || path_to_pdf == NULL) {
        return ERR_PARAMETER;
    }

    #ifdef _WIN32
        // convert path to wchar_t
        size_t out_size;
        size_t path_len;
        wchar_t *path_to_pdf_win;

        path_len = strlen(path_to_pdf) + 1;
        path_to_pdf_win = malloc(path_len * sizeof(wchar_t));
        if (path_to_pdf_win == NULL)
            return ERR_ALLOCATION;
        sigil_zeroize(path_to_pdf_win, path_len * sizeof(wchar_t));
        if (mbstowcs_s(&out_size,       // out ... characters converted
                       path_to_pdf_win, // out ... converted string
                       path_len,        // in  ... size of path_to_pdf_win
                       path_to_pdf,     // in  ... input string
                       path_len - 1     // in  ... max wide chars to store
           ) != 0)
        {
            free(path_to_pdf_win);
            return ERR_IO;
        }// MultiByteToWideChar TODO

        if (_wfopen_s(&pdf_file, path_to_pdf_win, L"rb") != 0) {
            free(path_to_pdf_win);
            return ERR_IO;
        }

        free(path_to_pdf_win);
    #else
        if ((pdf_file = fopen(path_to_pdf, "rb")) == NULL)
            return ERR_IO;
    #endif

    sgl->pdf_data.deallocation_info |= DEALLOCATE_FILE;

    return sigil_set_pdf_file(sgl, pdf_file);
}

sigil_err_t sigil_set_pdf_buffer(sigil_t *sgl, char *pdf_content, size_t size)
{
    if (sgl == NULL || pdf_content == NULL || size <= 0)
        return ERR_PARAMETER;

    sgl->pdf_data.buffer = pdf_content;
    sgl->pdf_data.size = size;

    return ERR_NONE;
}

sigil_err_t sigil_set_trusted_system(sigil_t *sgl)
{
    if (sgl == NULL)
        return ERR_PARAMETER;

    if (sgl->trusted_store == NULL)
        return ERR_OPENSSL;

    if (X509_STORE_set_default_paths(sgl->trusted_store) != 1)
        return ERR_OPENSSL;

    return ERR_NONE;
}

sigil_err_t sigil_set_trusted_file(sigil_t *sgl, const char *path_to_file)
{
    if (sgl == NULL || path_to_file == NULL)
        return ERR_PARAMETER;

    if (sgl->trusted_store == NULL)
        return ERR_OPENSSL;

    if (X509_STORE_load_locations(sgl->trusted_store, path_to_file, NULL) != 1)
        return ERR_OPENSSL;

    return ERR_NONE;
}

sigil_err_t sigil_set_trusted_dir(sigil_t *sgl, const char *path_to_dir)
{
    if (sgl == NULL || path_to_dir == NULL)
        return ERR_PARAMETER;

    if (sgl->trusted_store == NULL)
        return ERR_OPENSSL;

    if (X509_STORE_load_locations(sgl->trusted_store, NULL, path_to_dir) != 1)
        return ERR_OPENSSL;

    return ERR_NONE;
}

static sigil_err_t sigil_verify_adbe_x509_rsa_sha1(sigil_t *sgl)
{
    sigil_err_t err;

    err = load_certificates(sgl);
    if (err != ERR_NONE)
        return err;

    err = verify_signing_certificate(sgl);
    if (err != ERR_NONE)
        return err;

    err = load_digest(sgl);
    if (err != ERR_NONE)
        return err;

    return verify_digest(sgl, &(sgl->result_digest_comparison));
}

sigil_err_t sigil_verify(sigil_t *sgl)
{
    sigil_err_t err;

    // function parameter checks
    if (sgl == NULL)
        return ERR_PARAMETER;

    // process header - %PDF-<pdf_x>.<pdf_y>
    err = process_header(sgl);
    if (err != ERR_NONE)
        return err;

    // determine offset to the first cross-reference section
    err = read_startxref(sgl);
    if (err != ERR_NONE)
        return err;

    if (sgl->xref != NULL)
        xref_free(sgl->xref);
    sgl->xref = xref_init();
    if (sgl->xref == NULL)
        return ERR_ALLOCATION;

    sgl->xref->prev_section = sgl->offset_startxref;

    size_t max_file_updates = MAX_FILE_UPDATES;

    while (sgl->xref->prev_section > 0 && (max_file_updates--) > 0) {
        // go to the position of the beginning of next cross-reference section
        err = pdf_move_pos_abs(sgl, sgl->xref->prev_section);
        if (err != ERR_NONE)
            return err;

        sgl->xref->prev_section = 0;

        err = process_xref(sgl);
        if (err != ERR_NONE)
            return err;

        err = process_trailer(sgl);
        if (err != ERR_NONE)
            return err;
    }

    err = process_catalog_dictionary(sgl);
    if (err != ERR_NONE)
        return err;

    err = process_acroform(sgl);
    if (err != ERR_NONE)
        return err;

    if ((sgl->sig_flags & 0x01) == 0)
        return ERR_NO_SIGNATURE;

    err = find_sig_field(sgl);
    if (err != ERR_NONE)
        return err;

    err = process_sig_field(sgl);
    if (err != ERR_NONE)
        return err;

    err = process_sig_dict(sgl);
    if (err != ERR_NONE)
        return err;

    switch (sgl->subfilter_type) {
        case SUBFILTER_adbe_x509_rsa_sha1:
            err = sigil_verify_adbe_x509_rsa_sha1(sgl);
            if (err != ERR_NONE)
                return err;

            return ERR_NONE;
        default:
            return ERR_NOT_IMPLEMENTED;
    }

    return ERR_NONE;
}

sigil_err_t sigil_get_result(sigil_t *sgl, int *result)
{
    sigil_err_t err;
    int cert_res;
    int digest_res;

    if (sgl == NULL || result == NULL)
        return ERR_PARAMETER;

    *result = 0;

    switch (sgl->subfilter_type) {
        case SUBFILTER_adbe_x509_rsa_sha1:
            err = sigil_get_cert_validation_result(sgl, &cert_res);
            if (err != ERR_NONE)
                return err;

            err = sigil_get_data_integrity_result(sgl, &digest_res);
            if (err != ERR_NONE)
                return err;

            *result = (cert_res == CERT_STATUS_VERIFIED) &&
                      (digest_res == HASH_CMP_RESULT_MATCH);

            return ERR_NONE;
        default:
            return ERR_NOT_IMPLEMENTED;
    }
}

sigil_err_t sigil_get_cert_validation_result(sigil_t *sgl, int *result)
{
    if (sgl == NULL || result == NULL)
        return ERR_PARAMETER;

    *result = sgl->result_cert_verification;

    return ERR_NONE;
}

sigil_err_t sigil_get_data_integrity_result(sigil_t *sgl, int *result)
{
    if (sgl == NULL || result == NULL)
        return ERR_PARAMETER;

    *result = sgl->result_digest_comparison;

    return ERR_NONE;
}

sigil_err_t sigil_get_original_digest(sigil_t *sgl, ASN1_OCTET_STRING **digest)
{
    if (sgl == NULL || digest == NULL)
        return ERR_PARAMETER;

    if (sgl->digest_original == NULL)
        return ERR_NO_DATA;

    *digest = ASN1_OCTET_STRING_dup(sgl->digest_original);

    return ERR_NONE;
}

sigil_err_t sigil_get_computed_digest(sigil_t *sgl, ASN1_OCTET_STRING **digest)
{
    if (sgl == NULL || digest == NULL)
        return ERR_PARAMETER;

    if (sgl->digest_computed == NULL)
        return ERR_NO_DATA;

    *digest = ASN1_OCTET_STRING_dup(sgl->digest_computed);

    return ERR_NONE;
}

sigil_err_t sigil_get_subfilter(sigil_t *sgl, int *subfilter)
{
    if (sgl == NULL || subfilter == NULL)
        return ERR_PARAMETER;

    *subfilter = sgl->subfilter_type;

    return ERR_NONE;
}

static void range_free(range_t *range)
{
    if (range == NULL)
        return;

    range_free(range->next);

    free(range);
}

void cert_free(cert_t *cert)
{
    if (cert == NULL)
        return;

    cert_free(cert->next);

    if (cert->cert_hex != NULL)
        free(cert->cert_hex);

    if (cert->x509 != NULL)
        X509_free(cert->x509);

    free(cert);
}

void contents_free(sigil_t *sgl)
{
    if (sgl == NULL || sgl->contents == NULL)
        return;

    if (sgl->contents->contents_hex != NULL)
        free(sgl->contents->contents_hex);

    free(sgl->contents);
    sgl->contents = NULL;
}

void sigil_free(sigil_t **sgl)
{
    if (sgl == NULL || *sgl == NULL)
        return;

    if ((*sgl)->pdf_data.deallocation_info & DEALLOCATE_FILE) {
        fclose((*sgl)->pdf_data.file);
        (*sgl)->pdf_data.deallocation_info ^= DEALLOCATE_FILE;
    }
    if ((*sgl)->pdf_data.deallocation_info & DEALLOCATE_BUFFER) {
        free((*sgl)->pdf_data.buffer);
        (*sgl)->pdf_data.deallocation_info ^= DEALLOCATE_BUFFER;
    }

    if ((*sgl)->xref != NULL)
        xref_free((*sgl)->xref);


    if ((*sgl)->fields.capacity > 0) {
        for (size_t i = 0; i < (*sgl)->fields.capacity; i++) {
            if ((*sgl)->fields.entry[i] != NULL) {
                free((*sgl)->fields.entry[i]);
            }
        }

        if ((*sgl)->fields.entry != NULL)
            free((*sgl)->fields.entry);
    }

    if ((*sgl)->byte_range != NULL)
        range_free((*sgl)->byte_range);

    if ((*sgl)->certificates != NULL)
        cert_free((*sgl)->certificates);

    if ((*sgl)->contents != NULL)
        contents_free(*sgl);

    if ((*sgl)->digest_computed != NULL)
        ASN1_OCTET_STRING_free((*sgl)->digest_computed);

    if ((*sgl)->digest_algorithm != NULL)
        X509_ALGOR_free((*sgl)->digest_algorithm);

    if ((*sgl)->digest_original != NULL)
        ASN1_OCTET_STRING_free((*sgl)->digest_original);

    if ((*sgl)->trusted_store != NULL)
        X509_STORE_free((*sgl)->trusted_store);

    free(*sgl);
    *sgl = NULL;
}

const char *sigil_err_string(sigil_err_t err)
{
    switch (err) {
        case ERR_NONE:
            return "finished without any error";
        case ERR_ALLOCATION:
            return "ERROR during allocation";
        case ERR_PARAMETER:
            return "ERROR bad data between function parameters";
        case ERR_IO:
            return "ERROR during performing input/output operation";
        case ERR_PDF_CONTENT:
            return "ERROR unexpected data on input, probably corrupted PDF file";
        case ERR_NOT_IMPLEMENTED:
            return "ERROR this functionality is not currently available";
        case ERR_NO_DATA:
            return "ERROR no data available";
        case ERR_END_OF_DICT:
            return "ERROR end of dictionary occured while processing it's content";
        case ERR_NO_SIGNATURE:
            return "ERROR this file appears to be missing the signature";
        case ERR_OPENSSL:
            return "ERROR something bad happened inside of OpenSSL functionality";
        case ERR_DIGEST_TYPE:
            return "ERROR the signature is using not standard message digest";
        default:
            return "ERROR unknown";
    }
}

int sigil_sigil_self_test(int verbosity)
{
    sigil_err_t err;
    sigil_t *sgl = NULL;

    print_module_name("sigil", verbosity);

    // TEST: fn sigil_init
    print_test_item("fn sigil_init", verbosity);

    {
        sgl = NULL;
        err = sigil_init(&sgl);
        if (err != ERR_NONE || sgl == NULL)
            goto failed;

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: correct value of file size
    print_test_item("file size", verbosity);

    {
        sgl = test_prepare_sgl_path("test/EduLib__adbe.x509.rsa_sha1.pdf");
        if (sgl == NULL)
            goto failed;

        if (sgl->pdf_data.size != 60457)
            goto failed;

        // TODO test verification result

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);



    // TEST: fn sigil_verify with subfilter x509.rsa_sha1
    print_test_item("VERIFY x509.rsa_sha1", verbosity);

    {
        sgl = test_prepare_sgl_path("test/subtype_adbe.x509.rsa_sha1.pdf");
        if (sgl == NULL)
            goto failed;

        if (sigil_set_trusted_system(sgl) != ERR_NONE)
            goto failed;

        if (sigil_verify(sgl) != ERR_NONE)
            goto failed;

        // TODO test verification result

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // TEST: fn sigil_verify
    print_test_item("fn sigil_verify", verbosity);

    {
        sgl = test_prepare_sgl_path(
                "test/uznavany_bez_razitka_bez_revinfo_27_2_2012_CMS.pdf");
        if (sgl == NULL)
            goto failed;

        if (sigil_verify(sgl) != ERR_NONE || 1)
            goto failed;

        // TODO test verification result

        sigil_free(&sgl);
    }

    print_test_result(1, verbosity);

    // all tests done
    print_module_result(1, verbosity);

    return 0;

failed:
    if (sgl)
        sigil_free(&sgl);

    print_test_result(0, verbosity);
    print_module_result(0, verbosity);

    return 1;
}
