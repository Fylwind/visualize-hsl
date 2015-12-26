#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "color.h"
#include "util.h"

/** Transform the image into RGBA with 8-bit channels, regardless of what the
    original format was. */
void normalize_png(png_structp png, png_infop info)
{
    const png_byte bit_depth = png_get_bit_depth(png, info);
    const png_byte color_type = png_get_color_type(png, info);
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }
    if (bit_depth == 16) {
        png_set_strip_16(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE ||
        color_type == PNG_COLOR_TYPE_RGB) {
        png_set_filler(png, 0xff, PNG_FILLER_AFTER);
    }
    /* ensure png_get_rowbytes returns the updated value */
    png_read_update_info(png, info);
}

png_bytepp alloc_32bit_rows(size_t width, size_t height)
{
    png_bytepp rows = (png_bytepp)malloc(sizeof(*rows) * height);
    RF_X(!rows);
    if (height) {
        png_bytepp row;
        const size_t rowbytes = sizeof(**rows) * width * 4;
        rows[0] = (png_bytep)malloc(rowbytes * height);
        RF_X(!rows[0]);
        for (row = rows + 1; row != rows + height; ++row) {
            *row = *(row - 1) + rowbytes;
        }
    }
    return rows;
}

void free_rows(png_bytepp rows, size_t height)
{
    if (height) {
        free(rows[0]);
    }
    free(rows);
}

png_bytepp read_png(size_t *width, size_t *height,
                    const char *filename)
{
    FILE *file;
    png_structp png;
    png_infop info;
    png_bytepp rows;
    size_t rowbytes;

    file = fopen(filename, "rb");
    RF_X(!file);

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    RF_X(!png);

    info = png_create_info_struct(png);
    RF_X(!info);

    png_init_io(png, file);
    png_read_info(png, info);
    normalize_png(png, info);

    *width = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    rowbytes = png_get_rowbytes(png, info);
    assert(*width * 4 == rowbytes);

    rows = alloc_32bit_rows(*width, *height);
    png_read_image(png, rows);
    png_read_end(png, NULL);

    /* cleanup */
    png_destroy_read_struct(&png, &info, NULL);
    fclose(file);
    return rows;
}

void write_png(size_t width, size_t height,
               png_bytepp rows, const char *filename)
{
    FILE *file;
    png_structp png;
    png_infop info;

    file = fopen(filename, "wb");
    RF_X(!file);

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    RF_X(!png);

    info = png_create_info_struct(png);
    RF_X(!info);

    png_init_io(png, file);

    png_set_IHDR(png, info,
                 width, height, 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, info);
    png_write_image(png, rows);
    png_write_end(png, NULL);

    /* cleanup */
    png_destroy_write_struct(&png, &info);
    fclose(file);
}

typedef int64_t dist_type;

struct data {
    size_t width, height, avg_rgb_size;
    int64_t img_count;
    double *avg_rgb;
    dist_type *hsl_dist;
};

static const size_t hsl_dist_size = 361 * 257 * 256 * sizeof(dist_type);

static dist_type *calloc_dist(size_t size)
{
    dist_type *ptr = (dist_type *)calloc(size, 1);
    RF_X(!ptr);
    return ptr;
}

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

static void process_png(struct data *data, const char *filename)
{
    size_t width, height, x, y;
    png_bytepp rows;
    rows = read_png(&width, &height, filename);
    for (y = 0; y != height; ++y) {
        for (x = 0; x != width; ++x) {
            size_t r, g, b, h, s, l;
            r = rows[y][x * 4 + 0];
            g = rows[y][x * 4 + 1];
            b = rows[y][x * 4 + 2];
            discrete_rgb_to_hsl(&h, &s, &l, r, g, b);
            if (x < data->width && y < data->height) {
                data->avg_rgb[(y * data->width + x) * 3 + 0] += r;
                data->avg_rgb[(y * data->width + x) * 3 + 1] += g;
                data->avg_rgb[(y * data->width + x) * 3 + 2] += b;
            }
            data->hsl_dist[(h * 257 + s) * 256 + l] += 1;
        }
    }
    ++data->img_count;
    free_rows(rows, height);
}

static png_bytepp calc_avg_rgb(const struct data *self)
{
    size_t x, y, width = self->width, height = self->height;
    png_bytepp rows = alloc_32bit_rows(width, height);
    for (y = 0; y != height; ++y) {
        for (x = 0; x != width; ++x) {
            double r, g, b;
            r = self->avg_rgb[(y * width + x) * 3 + 0];
            g = self->avg_rgb[(y * width + x) * 3 + 1];
            b = self->avg_rgb[(y * width + x) * 3 + 2];
            r /= self->img_count;
            g /= self->img_count;
            b /= self->img_count;
            rows[y][x * 4 + 0] = r;
            rows[y][x * 4 + 1] = g;
            rows[y][x * 4 + 2] = b;
            rows[y][x * 4 + 3] = 0xff;
       }
    }
    return rows;
}

static int file_is_readable(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}

static void data_init(struct data *self, size_t width, size_t height)
{
    self->width = width;
    self->height = height;
    self->img_count = 0;
    self->avg_rgb_size = (sizeof(*self->avg_rgb) *
                          (3 * self->width * self->height));
    self->avg_rgb = (double *)calloc(self->avg_rgb_size, 1);
    self->hsl_dist = calloc_dist(hsl_dist_size);
}

static void data_reset(struct data *self)
{
    free(self->avg_rgb);
    free(self->hsl_dist);
}

static void data_read(struct data *self, const char *filename)
{
    FILE *f = fopen(filename, "rb");
    RF_X(!f);
    RF_X(rf_file_read(&self->img_count, sizeof(self->img_count), f));
    RF_X(rf_file_read(self->avg_rgb, self->avg_rgb_size, f));
    RF_X(rf_file_read(self->hsl_dist, hsl_dist_size, f));
    fclose(f);
}

static void data_write(const struct data *self, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    RF_X(!f);
    RF_X(rf_file_write(&self->img_count, sizeof(self->img_count), f));
    RF_X(rf_file_write(self->avg_rgb, self->avg_rgb_size, f));
    RF_X(rf_file_write(self->hsl_dist, hsl_dist_size, f));
    fclose(f);
}

static void usage(void)
{
    fprintf(
        stderr,
        "usage:\n"
        "  loadpngs in <width> <height> <data-file> <in-png>...\n"
        "  loadpngs out <width> <height> <data-file> <out-png> <out-hsl>\n"
    );
    fflush(stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    const char *data_filename;
    struct data data;

    if (argc < 6) {
        usage();
    }

    data_init(&data, atoi(argv[2]), atoi(argv[3]));
    data_filename = argv[4];

    if (!strcmp(argv[1], "in")) {
        char **arg;

        if (file_is_readable(data_filename)) {
            data_read(&data, data_filename);
        }
        for (arg = argv + 5; arg != argv + argc; ++arg) {
            process_png(&data, *arg);
        }
        data_write(&data, data_filename);

    } else if (!strcmp(argv[1], "out")) {
        png_bytepp rows;

        if (argc != 7) {
            usage();
        }

        data_read(&data, data_filename);

        /* calculate averaged RGB and save to PNG file */
        rows = calc_avg_rgb(&data);
        write_png(data.width, data.height, rows, argv[5]);
        free_rows(rows, data.height);

        /* extract the HSL distribution */
        RF_X(rf_writefile(data.hsl_dist, hsl_dist_size, argv[6]));

    }

    data_reset(&data);
    return 0;
}
