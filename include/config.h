/** @file
 *
 */

#ifndef PDF_SIGIL_CONFIG_H
#define PDF_SIGIL_CONFIG_H

/** @brief maximum bytes to read from the beginning of the file to look for
 *         the "%PDF-x.y"
 *
 */
#define HEADER_SEARCH_OFFSET        1024

/** @brief maximum bytes to read from the end of file to look for the "startxref"
 *
 */
#define XREF_SEARCH_OFFSET          1024

/** @brief capacity to choose for the first xref allocation
 *
 */
#define XREF_PREALLOCATION          10

/** @brief capacity to choose for the first allocation in array of fields
 *
 */
#define REF_ARRAY_PREALLOCATION     10

/** @brief capacity to choose for the first allocation of array for certificates
 *
 */
#define CERT_HEX_PREALLOCATION      1024

/** @brief capacity to choose for the first allocation of array for contents
 *
 */
#define CONTENTS_PREALLOCATION      1024

/** @brief threshold in bytes for loading whole file into buffer
 *
 */
#define THRESHOLD_FILE_BUFFERING    10485760

/** @brief maximum number of file updates, preventing forever loop in processing
 *         previous cross-reference sections (caused by cyclic links)
 *
 */
#define MAX_FILE_UPDATES            1024

/** @brief maximum size we give to hash function at once
 *
 */
#define HASH_UPDATE_SIZE            1024

/** @brief Tests for the config module
 *
 * @param verbosity output level - 0 means nothing, 1 prints module names with
 *                  the overall module result, and 2 prints also each test inside
 *                  of the module
 * @return 0 if success, 1 if failed
 */
int sigil_config_self_test(int verbosity);

#endif /* PDF_SIGIL_CONFIG_H */
