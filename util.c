#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void rf_x(int err, const char *file, unsigned long line)
{
    if (!err)
        return;
    fprintf(stderr, "%s:%lu: error %i\n", file, line, err);
    fflush(stderr);
    abort();
}

size_t rf_file_read(void *ptr, size_t count, FILE *stream)
{
    size_t n;
    char *p = (char *)ptr;
    for (; count > BUFSIZ; p += BUFSIZ) {
        n = fread(p, 1, BUFSIZ, stream);
        count -= n;
        if (n != BUFSIZ) {
            return count;
        }
    }
    n = fread(p, 1, count, stream);
    count -= n;
    return count;
}

size_t rf_file_write(const void *ptr, size_t count, FILE *stream)
{
    size_t n;
    char *p = (char *)ptr;
    for (; count > BUFSIZ; p += BUFSIZ) {
        n = fwrite(p, 1, BUFSIZ, stream);
        count -= n;
        if (n != BUFSIZ) {
            return count;
        }
    }
    n = fwrite(p, 1, count, stream);
    count -= n;
    return count;
}

int rf_readfile(void *ptr, size_t size, const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return 1;
    }
    if (rf_file_read(ptr, size, f)) {
        return 1;
    }
    if (fclose(f)) {
        return 1;
    }
    return 0;
}

int rf_writefile(void *ptr, size_t size, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f) {
        return 1;
    }
    if (rf_file_write(ptr, size, f)) {
        return 1;
    }
    if (fclose(f)) {
        return 1;
    }
    return 0;
}
