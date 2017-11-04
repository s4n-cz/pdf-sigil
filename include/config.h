#ifndef PDF_SIGIL_CONFIG_H
#define PDF_SIGIL_CONFIG_H


// maximum bytes to read from the beginning of file to look for the "%PDF-x.y"
#define HEADER_SEARCH_OFFSET    1024

// maximum bytes to read from the end of file to look for the "startxref"
#define XREF_SEARCH_OFFSET      1024

// validate values
int sigil_config_self_test(int quiet);

#endif /* PDF_SIGIL_CONFIG_H */
