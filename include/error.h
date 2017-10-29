#ifndef PDF_SIGIL_ERROR_H
#define PDF_SIGIL_ERROR_H

#include <stdio.h>  // FILE
#include <stdint.h> // uint32_t

#define ERR_NO           0x0000 // [_|_|_|x] 0000 0000
#define ERR_ALLOC        0x0001 // [_|_|_|x] 0000 0001
#define ERR_PARAM        0x0002 // [_|_|_|x] 0000 0010
#define ERR_IO           0x0004 // [_|_|_|x] 0000 0100
#define ERR_PDF_CONT     0x0008 // [_|_|_|x] 0000 1000
#define ERR_5            0x0010 // [_|_|_|x] 0001 0000
#define ERR_6            0x0020 // [_|_|_|x] 0010 0000
#define ERR_7            0x0040 // [_|_|_|x] 0100 0000
#define ERR_8            0x0080 // [_|_|_|x] 1000 0000
#define ERR_9            0x0100 // [_|_|x|_] 0000 0001
#define ERR_10           0x0200 // [_|_|x|_] 0000 0010
#define ERR_11           0x0400 // [_|_|x|_] 0000 0100
#define ERR_12           0x0800 // [_|_|x|_] 0000 1000
#define ERR_13           0x1000 // [_|_|x|_] 0001 0000
#define ERR_14           0x2000 // [_|_|x|_] 0010 0000
#define ERR_15           0x4000 // [_|_|x|_] 0100 0000
#define ERR_16           0x8000 // [_|_|x|_] 1000 0000

typedef uint32_t sigil_err_t;

const char *sigil_err_string(sigil_err_t err);

int sigil_error_self_test(int quiet);

#endif /* PDF_SIGIL_ERROR_H */
