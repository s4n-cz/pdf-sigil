#ifndef PDF_SIGIL_CONSTANTS_H
#define PDF_SIGIL_CONSTANTS_H


#define COLOR_RED          "\x1b[31m"
#define COLOR_GREEN        "\x1b[32m"
#define COLOR_RESET        "\x1b[0m"

#define KEYWORD_xref       0
#define KEYWORD_trailer    1

#define XREF_TYPE_UNSET    0
#define XREF_TYPE_TABLE    1
#define XREF_TYPE_STREAM   2

#define IN_USE_ENTRY       0
#define FREE_ENTRY         1

#define DICT_KEY_Size      0
#define DICT_KEY_Prev      1
#define DICT_KEY_Root      2
#define DICT_KEY_unknown   3

#define ERR_NO             0
#define ERR_ALLOC          1
#define ERR_PARAM          2
#define ERR_IO             3
#define ERR_PDF_CONT       4
#define ERR_NOT_IMPL       5
#define ERR_6              6
#define ERR_7              7
#define ERR_8              8
#define ERR_9              9
#define ERR_10            10
#define ERR_11            11
#define ERR_12            12
#define ERR_13            13
#define ERR_14            14
#define ERR_15            15
#define ERR_16            16

#endif /* PDF_SIGIL_CONSTANTS_H */
