#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/** Convenience macro for `rf_x`. */
#define RF_X(expr) rf_x((expr), __FILE__, (unsigned long)__LINE__)

/** Abort the program if `err` is nonzero. */
void rf_x(int err, const char *file, unsigned long line)
{
    if (!err)
        return;
    fprintf(stderr, "%s:%lu: error %i\n", file, line, err);
    fflush(stderr);
    abort();
}

/** Returns the number of bytes that could not be read. */
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

/** Returns the number of bytes that could not be written. */
size_t rf_file_write(void *ptr, size_t count, FILE *stream)
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

/** Find the minimum and maximum of 3 numbers. */
void fiminmax3(const double n[3], int *imin, int *imax) {
    if (n[0] < n[1]) {
        if (n[1] < n[2]) {
            /* 0 1 2 */
            *imin = 0;
            *imax = 2;
        } else {
            *imax = 1;
            if (n[0] < n[2]) {
                /* 0 2 1 */
                *imin = 0;
            } else {
                /* 2 0 1 */
                *imin = 2;
            }
        }
    } else {
        if (n[0] < n[2]) {
            /* 1 0 2 */
            *imin = 1;
            *imax = 2;
        } else {
            *imax = 0;
            if (n[1] < n[2]) {
                /* 1 2 0 */
                *imin = 1;
            } else {
                /* 2 1 0 */
                *imin = 2;
            }
        }
    }
}

/** Convert RGB to HSL.  All values lie in the range [0, 1], although
    round-off errors may cause the results to slightly exceed the range.
    Note: Hue can be NaN if chroma is zero; saturation can also be NaN if the
    color is black or white. */
void rgb_to_hsl(double r, double g, double b, double *h, double *s, double *l)
{
    const double rgb[] = {r, g, b};
    int iminrgb, imaxrgb;
    double minrgb, maxrgb, chroma, hue, saturation, lightness;

    /* determine ordering of r, g, and b */
    fiminmax3(rgb, &iminrgb, &imaxrgb);
    minrgb = rgb[iminrgb];
    maxrgb = rgb[imaxrgb];

    /* compute chroma and thus hue */
    chroma = maxrgb - minrgb;
    switch (imaxrgb) {
    case 0:
        hue = (g - b) / chroma;
        if (hue < 0) {
            hue += 6.;
        }
        break;
    case 1:
        hue = (b - r) / chroma + 2.;
        break;
    case 2:
        hue = (r - g) / chroma + 4.;
        break;
    }
    hue *= 1 / 6.;

    /* compute lightness and saturation */
    lightness = minrgb + maxrgb;
    saturation = chroma / (1. - fabs(lightness - 1.));
    lightness *= .5;

    /* set the output variables if provided */
    if (h) {
        *h = hue;
    }
    if (s) {
        *s = saturation;
    }
    if (l) {
        *l = lightness;
    }
}

size_t clamp_size(size_t x, size_t low, size_t high)
{
    if (x < low) {
        return low;
    } else if (x > high) {
        return high;
    } else {
        return x;
    }
}

/* ------------------------------------------------------------------------ */

typedef int64_t dist_type;

static const size_t rgb_dist_size = 256 * 256 * 256 * sizeof(dist_type);
static const size_t hsl_dist_size = 361 * 257 * 256 * sizeof(dist_type);

static dist_type *calloc_dist(size_t size)
{
    dist_type *ptr = (dist_type *)calloc(size, 1);
    RF_X(!ptr);
    return ptr;
}

static void discrete_rgb_to_hsl(size_t r, size_t g, size_t b,
                                size_t *h, size_t *s, size_t *l)
{
    double fh, fs, fl;
    rgb_to_hsl(r / 255., g / 255., b / 255., &fh, &fs, &fl);
    if (isfinite(fh)) {
        *h = clamp_size((size_t)(fh * 360.), 0, 359);
    } else {
        *h = 360;
    }
    if (isfinite(fs)) {
        *s = clamp_size((size_t)(fs * 255.), 0, 255);
    } else {
        *s = 256;
    }
    RF_X(!isfinite(fl));
    *l = clamp_size((size_t)(fl * 255.), 0, 255);
}

static void rgb_to_hsl_dist(dist_type *rgb_dist, dist_type *hsl_dist)
{
    size_t r, g, b;
    for (r = 0; r != 256; ++r)
    for (g = 0; g != 256; ++g)
    for (b = 0; b != 256; ++b) {
        size_t h, s, l;
        discrete_rgb_to_hsl(r, g, b, &h, &s, &l);
        hsl_dist[(h * 257 + s) * 256 + l] += rgb_dist[(r * 256 + g) * 256 + b];
    }
}

static void parse_args(int argc, char **argv,
                       const char **in_filename,
                       const char **out_filename)
{
    /* expect exactly three arguments */
    RF_X(argc != 3);
    *in_filename = argv[1];
    *out_filename = argv[2];
}

int main(int argc, char **argv)
{
    dist_type *rgb_dist, *hsl_dist;
    const char *in_filename, *out_filename;

    parse_args(argc, argv, &in_filename, &out_filename);

    rgb_dist = calloc_dist(rgb_dist_size);
    hsl_dist = calloc_dist(hsl_dist_size);

    RF_X(rf_readfile(rgb_dist, rgb_dist_size, in_filename));

    rgb_to_hsl_dist(rgb_dist, hsl_dist);

    RF_X(rf_writefile(hsl_dist, hsl_dist_size, out_filename));

    free(hsl_dist);
    free(rgb_dist);
    return 0;
}
