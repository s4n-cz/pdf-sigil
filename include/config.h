#ifndef PDF_SIGIL_CONFIG_H
#define PDF_SIGIL_CONFIG_H


// maximum bytes to read from the beginning of file to look for the "%PDF-x.y"
#define HEADER_SEARCH_OFFSET        1024

// maximum bytes to read from the end of file to look for the "startxref"
#define XREF_SEARCH_OFFSET          1024

// capacity to choose for the first xref allocation
#define XREF_PREALLOCATION          10

// capacity to choose for the first allocation in array of fields
#define REF_ARRAY_PREALLOCATION     10

// capacity to choose for the first allocation of array for certificates
#define CERT_HEX_PREALLOCATION      1024

// capacity to choose for the first allocation of array for contents
#define CONTENTS_PREALLOCATION      1024

// threshold in bytes for loading whole file into buffer
#define THRESHOLD_FILE_BUFFERING    10485760

// maximum number of file updates, preventing forever loop in processing
// previous cross-reference sections (caused by cyclic links)
#define MAX_FILE_UPDATES            1024

// validate values
int sigil_config_self_test(int verbosity);

#endif /* PDF_SIGIL_CONFIG_H */
