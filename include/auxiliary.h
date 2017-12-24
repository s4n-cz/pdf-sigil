#ifndef PDF_SIGIL_AUXILIARY_H
#define PDF_SIGIL_AUXILIARY_H

#include <stdlib.h>

#ifndef CHAR_T
#define CHAR_T
typedef char char_t;
#endif /* CHAR_T */


#define COLOR_RED        "\x1b[31m"
#define COLOR_GREEN      "\x1b[32m"
#define COLOR_RESET      "\x1b[0m"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Shouldn't be optimized out by the compiler
void sigil_zeroize(void *a, size_t bytes);

int is_digit(const char_t c);

int is_whitespace(const char_t c);

void print_module_name(const char *module_name, int verbosity);
void print_module_result(int result, int verbosity);
void print_test_item(const char *test_name, int verbosity);
void print_test_result(int result, int verbosity);

int sigil_auxiliary_self_test(int verbosity);

#endif /* PDF_SIGIL_AUXILIARY_H */
