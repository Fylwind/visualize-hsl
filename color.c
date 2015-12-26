#include <math.h>
#include "color.h"

double rf_ffloormod(double x, double y)
{
    double r = fmod(x, y);
    if (signbit(r) != signbit(y)) {
        r += y;
    }
    return r;
}

void rf_fiminmax3(int *imin, int *imax, const double n[3]) {
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

void rf_rgb_to_hsl(double *h, double *s, double *l,
                   double r, double g, double b)
{
    const double rgb[] = {r, g, b};
    int iminrgb, imaxrgb;
    double minrgb, maxrgb, chroma, hue, saturation, lightness;

    /* determine ordering of r, g, and b */
    rf_fiminmax3(&iminrgb, &imaxrgb, rgb);
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
    hue *= 1. / 6.;

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

void rf_hsl_to_rgb(double *r, double *g, double *b,
                   double h, double s, double l)
{
    const int definite_hue = isfinite(h);
    double red, green, blue, chroma;

    if (definite_hue) {
        h = rf_ffloormod(h, 1.);
        h *= 6.;
        chroma = (1. - fabs(2. * l - 1.)) * s;
    } else {
        chroma = 0;
    }

    red = green = blue = l - .5 * chroma;

    if (definite_hue) {
        const double mid = chroma * (1. - fabs(fmod(h, 2.) - 1.));
        switch ((unsigned)h) {
        case 0:
            red += chroma;
            green += mid;
            break;
        case 1:
            red += mid;
            green += chroma;
            break;
        case 2:
            green += chroma;
            blue += mid;
            break;
        case 3:
            green += mid;
            blue += chroma;
            break;
        case 4:
            red += mid;
            blue += chroma;
            break;
        case 5:
            red += chroma;
            blue += mid;
            break;
        }
    }

    if (r) {
        *r = red;
    }
    if (g) {
        *g = green;
    }
    if (b) {
        *b = blue;
    }
}
