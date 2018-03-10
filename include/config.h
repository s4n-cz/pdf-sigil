#ifndef PDF_SIGIL_CONFIG_H
#define PDF_SIGIL_CONFIG_H


// maximum bytes to read from the beginning of file to look for the "%PDF-x.y"
#define HEADER_SEARCH_OFFSET        1024

// maximum bytes to read from the end of file to look for the "startxref"
#define XREF_SEARCH_OFFSET          1024

// capacity to choose for the first xref allocation
#define XREF_PREALLOCATION          10

// threshold in bytes for loading whole file into buffer
#define THRESHOLD_FILE_BUFFERING    10485760

// maximum number of file updates, preventing forever loop in processing
// previous cross-reference sections (caused by cyclic links)
#define MAX_FILE_UPDATES            1024

// validate values
int sigil_config_self_test(int verbosity);

#endif /* PDF_SIGIL_CONFIG_H */
