#ifndef PDF_SIGIL_ERROR_H
#define PDF_SIGIL_ERROR_H

#include <stdio.h>  // FILE
#include <stdint.h> // uint32_t


#define ERR_NO           0x0000 /* ... | 0000 0000 | 0000 0000 */
#define ERR_ALLOC        0x0001 /* ... | 0000 0000 | 0000 0001 */
#define ERR_PARAM        0x0002 /* ... | 0000 0000 | 0000 0010 */
#define ERR_IO           0x0004 /* ... | 0000 0000 | 0000 0100 */
#define ERR_PDF_CONT     0x0008 /* ... | 0000 0000 | 0000 1000 */
#define ERR_5            0x0010 /* ... | 0000 0000 | 0001 0000 */
#define ERR_6            0x0020 /* ... | 0000 0000 | 0010 0000 */
#define ERR_7            0x0040 /* ... | 0000 0000 | 0100 0000 */
#define ERR_8            0x0080 /* ... | 0000 0000 | 1000 0000 */
#define ERR_9            0x0100 /* ... | 0000 0001 | 0000 0000 */
#define ERR_10           0x0200 /* ... | 0000 0010 | 0000 0000 */
#define ERR_11           0x0400 /* ... | 0000 0100 | 0000 0000 */
#define ERR_12           0x0800 /* ... | 0000 1000 | 0000 0000 */
#define ERR_13           0x1000 /* ... | 0001 0000 | 0000 0000 */
#define ERR_14           0x2000 /* ... | 0010 0000 | 0000 0000 */
#define ERR_15           0x4000 /* ... | 0100 0000 | 0000 0000 */
#define ERR_16           0x8000 /* ... | 1000 0000 | 0000 0000 */

typedef uint32_t sigil_err_t;

void sigil_err_write(FILE *out, sigil_err_t err);

int sigil_error_self_test(int quiet);

#endif /* PDF_SIGIL_ERROR_H */
