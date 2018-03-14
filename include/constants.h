#ifndef PDF_SIGIL_CONSTANTS_H
#define PDF_SIGIL_CONSTANTS_H


#ifdef __unix__
    #define COLOR_RED         "\x1b[31m"
    #define COLOR_GREEN       "\x1b[32m"
    #define COLOR_RESET       "\x1b[0m"
#else
    #define COLOR_RED         ""
    #define COLOR_GREEN       ""
    #define COLOR_RESET       ""
#endif


#define XREF_TYPE_UNSET       0
#define XREF_TYPE_TABLE       1
#define XREF_TYPE_STREAM      2

#define DICT_KEY_UNKNOWN      0
#define DICT_KEY_Size         1
#define DICT_KEY_Prev         2
#define DICT_KEY_Root         3
#define DICT_KEY_AcroForm     4
#define DICT_KEY_Fields       5
#define DICT_KEY_SigFlags     6
#define DICT_KEY_FT           7
#define DICT_KEY_V            8
#define DICT_KEY_SubFilter    9
#define DICT_KEY_Cert         10
#define DICT_KEY_Contents     11
#define DICT_KEY_ByteRange    12

#define SUBFILTER_UNKNOWN               0
#define SUBFILTER_adbe_x509_rsa_sha1    1

#define CERT_STATUS_UNKNOWN   0
#define CERT_STATUS_VERIFIED  1
#define CERT_STATUS_FAILED    2

#define DEALLOCATE_FILE       0x01
#define DEALLOCATE_BUFFER     0x02

#define ERR_NO                0
#define ERR_ALLOCATION        1
#define ERR_PARAMETER         2
#define ERR_IO                3
#define ERR_PDF_CONTENT       4
#define ERR_NOT_IMPLEMENTED   5
#define ERR_NO_DATA           6
#define ERR_END_OF_DICT       7
#define ERR_NO_SIGNATURE      8
#define ERR_OPENSSL           9

#endif /* PDF_SIGIL_CONSTANTS_H */
