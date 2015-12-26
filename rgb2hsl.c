#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "color.h"
#include "util.h"

typedef int64_t dist_type;

static const size_t rgb_dist_size = 256 * 256 * 256 * sizeof(dist_type);
static const size_t hsl_dist_size = 361 * 257 * 256 * sizeof(dist_type);

static size_t clamp_size(size_t x, size_t low, size_t high)
{
    if (x < low) {
        return low;
    } else if (x > high) {
        return high;
    } else {
        return x;
    }
}

static dist_type *calloc_dist(size_t size)
{
    dist_type *ptr = (dist_type *)calloc(size, 1);
    RF_X(!ptr);
    return ptr;
}

static void discrete_rgb_to_hsl(size_t *h, size_t *s, size_t *l,
                                size_t r, size_t g, size_t b)
{
    double fh, fs, fl;
    rf_rgb_to_hsl(&fh, &fs, &fl, r / 255., g / 255., b / 255.);
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
        discrete_rgb_to_hsl(&h, &s, &l, r, g, b);
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
