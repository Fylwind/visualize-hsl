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

/** Convert RGB to HSL.  R, G, B, S, and L are from [0, 1], while H is from
    [0, 360).  Note: if chroma is zero, then hue is NaN.  Saturation can also
    be NaN if it's white or black. */
void rgb_to_hsl(double r, double g, double b, double *h, double *s, double *l)
{
    const double rgb[] = {r, g, b};
    int iminrgb, imaxrgb;
    double minrgb, maxrgb, chroma, hue, saturation, lightness;
    fiminmax3(rgb, &iminrgb, &imaxrgb);
    minrgb = rgb[iminrgb];
    maxrgb = rgb[imaxrgb];
    chroma = maxrgb - minrgb;
    switch (imaxrgb) {
    case 0:
        hue = (g - b) / chroma;
        if (hue < 0) {
            hue += 6;
        }
        break;
    case 1:
        hue = (b - r) / chroma + 2;
        break;
    case 2:
        hue = (r - g) / chroma + 4;
        break;
    }
    hue *= 60;
    lightness = minrgb + maxrgb;
    saturation = chroma / (1 - fabs(lightness - 1));
    lightness *= .5;
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

int main(int argc, char **argv)
{
    size_t rgb_dist_size, hsl_dist_size, r, g, b;
    int64_t *rgb_dist, *hsl_dist;
    FILE *f;
    const char *in_filename, *out_filename;

    RF_X(argc != 3);

    in_filename = argv[1];
    out_filename = argv[2];

    rgb_dist_size = 256 * 256 * 256 * sizeof(*rgb_dist);
    rgb_dist = (int64_t *)malloc(rgb_dist_size);
    RF_X(!rgb_dist);
    hsl_dist_size = 361 * 257 * 256 * sizeof(*hsl_dist);
    hsl_dist = (int64_t *)calloc(hsl_dist_size, 1);
    RF_X(!hsl_dist);

    f = fopen(in_filename, "rb");
    RF_X(!f);
    RF_X(!!rf_file_read(rgb_dist, rgb_dist_size, f));
    fclose(f);

    for (r = 0; r != 256; ++r)
    for (g = 0; g != 256; ++g)
    for (b = 0; b != 256; ++b) {
        size_t ih, is, il;
        double h, s, l;
        int64_t n = rgb_dist[(r * 256 + g) * 256 + b];
        rgb_to_hsl(r / 255., g / 255., b / 255., &h, &s, &l);
        if (isfinite(h)) {
            ih = clamp_size((size_t)h, 0, 359);
        } else {
            ih = 360;
        }
        if (isfinite(s)) {
            is = clamp_size((size_t)(s * 255), 0, 255);
        } else {
            is = 256;
        }
        RF_X(!isfinite(l));
        il = clamp_size((size_t)(l * 255), 0, 255);
        hsl_dist[(ih * 257 + is) * 256 + il] += n;
    }

    f = fopen(out_filename, "wb");
    RF_X(!f);
    RF_X(!!rf_file_write(hsl_dist, hsl_dist_size, f));
    fclose(f);

    free(hsl_dist);
    free(rgb_dist);
    return 0;
}
