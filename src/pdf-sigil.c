#include <stdio.h>
#include <string.h>
#include <sigil.h>
#include <constants.h>

void print_banner(void)
{
    fprintf(stderr,
        "                                            \n"
        "  ____  ____  _____    ____  _       _ _    \n"
        " |  _ \\|  _ \\|  ___|  / ___|(_) __ _(_) | \n"
        " | |_) | | | | |_ ____\\___ \\| |/ _` | | | \n"
        " |  __/| |_| |  _|_____|__) | | (_| | | |   \n"
        " |_|   |____/|_|      |____/|_|\\__, |_|_|  \n"
        "                               |___/        \n"
        "                                            \n"
        " ========================================== \n"
        "                                            \n");
}

void print_help(void)
{
    fprintf(stderr,
            " OPTIONS                                                         \n"
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
    int ret_code = 1;
    int help = 0;
    int quiet = 0;
    int trusted_system = 0;
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
        fprintf(stderr, " ERROR initialize sigil context\n");
        goto end;
    }

    // set PDF file for the verification
    if (sigil_set_pdf_path(sgl, file) != ERR_NONE) {
        fprintf(stderr, " ERROR with provided file\n");
        goto end;
    }

    // set trusted CA certificates
    if (trusted_system) {
        if (sigil_set_trusted_system(sgl) != ERR_NONE) {
            fprintf(stderr, " ERROR setting trusted certificates\n");
            goto end;
        }
    } else if (trusted_file != NULL) {
        if (sigil_set_trusted_file(sgl, trusted_file) != ERR_NONE) {
            fprintf(stderr, " ERROR setting trusted certificates\n");
            goto end;
        }
    } else if (trusted_dir != NULL) {
        if (sigil_set_trusted_dir(sgl, trusted_dir) != ERR_NONE) {
            fprintf(stderr, " ERROR setting trusted certificates\n");
            goto end;
        }
    }

    // verify and save the result to the context
    err = sigil_verify(sgl);
    if (err != ERR_NONE) {
        if (err == ERR_NOT_IMPLEMENTED) {
            fprintf(stderr, " ERROR file uses feature that is not implemented\n");
        } else {
            fprintf(stderr, " ERROR obtaining verification result from the context\n");
        }
        goto end;
    }

    err = sigil_get_result(sgl, &result);
    if (err != ERR_NONE) {
        if (err == ERR_NOT_IMPLEMENTED) {
            fprintf(stderr, " ERROR file uses feature that is not implemented\n");
        } else {
            fprintf(stderr, " ERROR obtaining verification result from the context\n");
        }
        goto end;
    }

    // print results
    printf(" VERIFICATION RESULTS\n");

    if (result == VERIFY_SUCCESS) {
        printf("     status: verification successful\n");
        ret_code = 0;
    } else {
        printf("     status: verification failed\n");
    }

end:
    if (sgl != NULL)
        sigil_free(&sgl);

    return ret_code;
}