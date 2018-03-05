#ifndef PDF_SIGIL_CONSTANTS_H
#define PDF_SIGIL_CONSTANTS_H


#define COLOR_RED          "\x1b[31m"
#define COLOR_GREEN        "\x1b[32m"
#define COLOR_RESET        "\x1b[0m"

#define KEYWORD_UNSET      0
#define KEYWORD_xref       1
#define KEYWORD_trailer    2

#define XREF_TYPE_UNSET    0
#define XREF_TYPE_TABLE    1
#define XREF_TYPE_STREAM   2

#define IN_USE_ENTRY       0
#define FREE_ENTRY         1

#define DICT_KEY_UNKNOWN   0
#define DICT_KEY_Size      1
#define DICT_KEY_Prev      2
#define DICT_KEY_Root      3

#define DEALLOCATE_FILE    0x01
#define DEALLOCATE_BUFFER  0x02

#define ERR_NO                0
#define ERR_ALLOCATION        1
#define ERR_PARAMETER         2
#define ERR_IO                3
#define ERR_PDF_CONTENT       4
#define ERR_NOT_IMPLEMENTED   5
#define ERR_NO_DATA           6
#define ERR_END_OF_DICT       7

#endif /* PDF_SIGIL_CONSTANTS_H */
