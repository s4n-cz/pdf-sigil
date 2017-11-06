#ifndef PDF_SIGIL_AUXILIARY_H
#define PDF_SIGIL_AUXILIARY_H

#include <stdlib.h>


#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Shouldn't be optimized out by the compiler
void sigil_zeroize(void *a, size_t cnt);

int is_digit(const char c);

int is_whitespace(const char c);

int sigil_auxiliary_self_test(int quiet);

#endif /* PDF_SIGIL_AUXILIARY_H */
