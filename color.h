#ifndef G_4NJL3S6P3PNHFIDZPZLHGVMAXCF2Z
#define G_4NJL3S6P3PNHFIDZPZLHGVMAXCF2Z
#ifdef __cplusplus
extern "C" {
#endif

/** Calculate the floored modulus. */
double rf_ffloormod(double x, double y);

/** Find the minimum and maximum of 3 numbers. */
void rf_fiminmax3(int *imin, int *imax, const double n[3]);

/** Convert RGB to HSL.  All values lie in the range [0, 1], although
    round-off errors may cause the results to slightly exceed the range.
    Note: Hue can be NaN if chroma is zero; saturation can also be NaN if the
    color is black or white. */
void rf_rgb_to_hsl(double *h, double *s, double *l,
                   double r, double g, double b);

/** Convert HSL to RGB.  See docs for `rf_rgb_to_hsl` for details. */
void rf_rgb_to_hsl(double *r, double *g, double *b,
                   double h, double s, double l);

#ifdef __cplusplus
}
#endif
#endif
