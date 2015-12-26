#ifndef G_EDTG4AWY5ZAK2PMZXRXKYEFME7EUQ
#define G_EDTG4AWY5ZAK2PMZXRXKYEFME7EUQ
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/** Convenience macro for `rf_x`. */
#define RF_X(expr) rf_x((expr), __FILE__, (unsigned long)__LINE__)

/** Abort the program if `err` is nonzero. */
void rf_x(int err, const char *file, unsigned long line);

/** Returns the number of bytes that could not be read. */
size_t rf_file_read(void *ptr, size_t count, FILE *stream);

/** Returns the number of bytes that could not be written. */
size_t rf_file_write(const void *ptr, size_t count, FILE *stream);

int rf_readfile(void *ptr, size_t size, const char *filename);

int rf_writefile(void *ptr, size_t size, const char *filename);

#ifdef __cplusplus
}
#endif
#endif
