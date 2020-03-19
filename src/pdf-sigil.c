#include <stdio.h>
#include <string.h>
#include <sigil.h>
#include <constants.h>

#define COLOR_CYAN        "\x1b[36m"

void print_banner(void)
{
    fprintf(stderr, COLOR_CYAN
        "                                            \n"
        "  ____  ____  _____    ____  _       _ _    \n"
        " |  _ \\|  _ \\|  ___|  / ___|(_) __ _(_) | \n"
        " | |_) | | | | |_ ____\\___ \\| |/ _` | | | \n"
        " |  __/| |_| |  _|_____|__) | | (_| | | |   \n"
        " |_|   |____/|_|      |____/|_|\\__, |_|_|  \n"
        "                               |___/        \n"
        "                                            \n"COLOR_RESET
        " ========================================   \n"
        "                                            \n");
}

void print_help(void)
{
    fprintf(stderr,
            " OPTIONS                                                         \n"
            "     -ci, --cert-info                                            \n"
            "         Output detail information about signing certificate     \n"
            "     -f, --file                                                  \n"
            "         PDF file with a digital signature for the verification. \n"
            "     -h, --help                                                  \n"
            "         Output a program usage message and exit.                \n"
            "     -q, --quiet                                                 \n"
            "         Do not print anything to standard/error output.         \n"
            "     -td, --trusted-dir                                          \n"
            "         Load all the certificates from a specified folder to a  \n"
            "         storage of the trusted certificates. The certificates   \n"
            "         inside of the folder must be in a hashed form as        \n"
            "         documented in OpenSSL function X509_LOOKUP_hash_dir.    \n"
            "     -tf, --trusted-file                                         \n"
            "         Load the certificate from provided file to a storage of \n"
            "         the trusted certificates.                               \n"
            "     -ts, --trusted-system                                       \n"
            "         Use the system storage of the trusted certificates for  \n"
            "         the verification.                                       \n"
            "                                                                 \n"
            " EXIT STATUS                                                     \n"
            "     0 ... the provided file was successfuly verified            \n"
            "     1 ... the signature is invalid/could not be verified/other  \n"
            "           error occured                                         \n"
    );
}

int main(int argc, char *argv[])
{
    sigil_t *sgl = NULL;
    sigil_err_t err;
    int result = VERIFY_FAILED;
    int result_integrity = HASH_CMP_RESULT_UNKNOWN;
    int result_certificate = CERT_STATUS_UNKNOWN;
    int ret_code = 1;
    int help = 0;
    int quiet = 0;
    int trusted_system = 0;
    int cert_info = 0;
    const char *trusted_file = NULL;
    const char *trusted_dir = NULL;
    const char *file = NULL;

    // process parameters from the command line
    for (int pos = 1; pos < argc; pos++) {
        if (strcmp(argv[pos], "-h") == 0 || strcmp(argv[pos], "--help") == 0) {
            help = 1;
            break;
        } else if (strcmp(argv[pos], "-q") == 0 || strcmp(argv[pos], "--quiet") == 0) {
            quiet = 1;
        } else if (strcmp(argv[pos], "-ts") == 0 || strcmp(argv[pos], "--trusted-system") == 0) {
            trusted_system = 1;
        } else if (strcmp(argv[pos], "-tf") == 0 || strcmp(argv[pos], "--trusted-file") == 0) {
            if (++pos >= argc) {
                break;
            }
            trusted_file = argv[pos];
        } else if (strcmp(argv[pos], "-td") == 0 || strcmp(argv[pos], "--trusted-dir") == 0) {
            if (++pos >= argc) {
                break;
            }
            trusted_dir = argv[pos];
        } else if (strcmp(argv[pos], "-f") == 0 || strcmp(argv[pos], "--file") == 0) {
            if (++pos >= argc) {
                break;
            }
            file = argv[pos];
        } else if (strcmp(argv[pos], "-ci") == 0 || strcmp(argv[pos], "--cert-info") == 0) {
            cert_info = 1;
        } else {
            if (!quiet) {
                fprintf(stderr, COLOR_RED
                        "ERROR unknown parameter: "COLOR_RESET"%s\n", argv[pos]);
                print_banner();
            }
            goto end;
        }
    }

    if (!quiet)
        print_banner();

    if (help) {
        if (!quiet)
            print_help();
        goto end;
    }

    if (file == NULL) {
        if (!quiet)
            print_help();
        goto end;
    }

    // initialize sigil context
    if (sigil_init(&sgl) != ERR_NONE) {
        if (!quiet) {
            fprintf(stderr, COLOR_RED
                    " ERROR initialize sigil context\n"COLOR_RESET);
        }
        goto end;
    }

    // set PDF file for the verification
    if (sigil_set_pdf_path(sgl, file) != ERR_NONE) {
        if (!quiet) {
            fprintf(stderr, COLOR_RED
                    " ERROR with provided file\n"COLOR_RESET);
        }
        goto end;
    }

    // set trusted CA certificates
    if (trusted_system) {
        if (sigil_set_trusted_system(sgl) != ERR_NONE) {
            if (!quiet) {
                fprintf(stderr, COLOR_RED
                        " ERROR setting trusted certificates\n"COLOR_RESET);
            }
            goto end;
        }
    } else if (trusted_file != NULL) {
        if (sigil_set_trusted_file(sgl, trusted_file) != ERR_NONE) {
            if (!quiet) {
                fprintf(stderr, COLOR_RED
                        " ERROR setting trusted certificates\n"COLOR_RESET);
            }
            goto end;
        }
    } else if (trusted_dir != NULL) {
        if (sigil_set_trusted_dir(sgl, trusted_dir) != ERR_NONE) {
            if (!quiet) {
                fprintf(stderr, COLOR_RED
                        " ERROR setting trusted certificates\n"COLOR_RESET);
            }
            goto end;
        }
    }

    // verify and save the result to the context
    err = sigil_verify(sgl);
    if (err != ERR_NONE) {
        if (!quiet) {
            if (err == ERR_NOT_IMPLEMENTED) {
                fprintf(stderr, COLOR_RED
                        " ERROR Unable to verify file: Uses feature that is not implemented\n"COLOR_RESET);
            } else {
                fprintf(stderr, COLOR_RED
                        " ERROR Unable to object verification result for file from the context\n"COLOR_RESET);
            }
        }
        goto end;
    }

    err = sigil_get_result(sgl, &result);
    if (err != ERR_NONE) {
        if (!quiet) {
            if (err == ERR_NOT_IMPLEMENTED) {
                fprintf(stderr, COLOR_RED
                        " ERROR file uses feature that is not implemented\n"COLOR_RESET);
            } else {
                fprintf(stderr, COLOR_RED
                        " ERROR obtaining verification result from the context\n"COLOR_RESET);
            }
        }
        goto end;
    }

    if (sigil_get_data_integrity_result(sgl, &result_integrity) != ERR_NONE && !quiet) {
        fprintf(stderr, COLOR_RED
                " ERROR failed to obtain data integrity result\n"COLOR_RESET);
    }

    if (sigil_get_cert_validation_result(sgl, &result_certificate) != ERR_NONE && !quiet) {
        fprintf(stderr, COLOR_RED
                " ERROR failed to obtain certificate validation result\n"COLOR_RESET);
    }

    // print verification result
    if (result == VERIFY_SUCCESS) {
        if (!quiet)
            printf(COLOR_GREEN" VERIFICATION SUCCESSFUL\n\n"COLOR_RESET);
        ret_code = 0;
    } else {
        if (!quiet)
            printf(COLOR_RED" VERIFICATION FAILED\n\n"COLOR_RESET);
    }

    // print verification details
    if (!quiet) {
        printf("     %-20s", "subfilter:");
        sigil_print_subfilter(sgl);
        printf("\n");
        printf("     %-20s", "hash function:");
        sigil_print_hash_fn(sgl);
        printf("\n\n");
        printf("     DATA INTEGRITY\n");
        printf("     --------------\n");
        printf("     %-20s", "original digest:");
        sigil_print_original_digest(sgl);
        printf("\n");
        printf("     %-20s", "computed digest:");
        sigil_print_computed_digest(sgl);
        printf("\n");
        printf("     %-20s", "digest match:");
        switch (result_integrity) {
            case HASH_CMP_RESULT_MATCH:
                printf(COLOR_GREEN"YES\n"COLOR_RESET);
                break;
            case HASH_CMP_RESULT_DIFFER:
                printf(COLOR_RED"NO\n"COLOR_RESET);
                break;
            default:
                printf(COLOR_RED"UNKNOWN\n"COLOR_RESET);
                break;
        }
        printf("\n");

        printf("     CERTIFICATE\n");
        printf("     -----------\n");
        printf("     %-20s", "verified:");
        switch (result_certificate) {
            case CERT_STATUS_VERIFIED:
                printf(COLOR_GREEN"YES\n"COLOR_RESET);
                break;
            case CERT_STATUS_FAILED:
                printf(COLOR_RED"NO\n"COLOR_RESET);
                break;
            default:
                printf(COLOR_RED"UNKNOWN\n"COLOR_RESET);
                break;
        }
        printf("\n");
        if (cert_info)
            sigil_print_cert_info(sgl);
    }

    end:
    if (sgl != NULL)
        sigil_free(&sgl);

    return ret_code;
}
