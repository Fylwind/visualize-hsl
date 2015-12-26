# Visualizing HSL color distributions

## npy2dat

Converts an input array in NumPy format into a raw binary file composed of
64-bit signed integers.  The dimensions of the array are not stored.

## rgb2hsl

Converts an RGB distribution into an HSL distribution.

The input file is a raw binary file containing a 256 × 256 × 256 array of RGB
frequencies (with B being the fastest index):

    int64_t rgb_dist[256][256][256];

As a distribution, the value of `rgb_dist[r][g][b]` indicates the number of
occurrences (frequency) of the color `(r, g, b)`.

The output file is a raw binary file containing a 361 × 257 × 256 array of HSL
frequencies (with L being the fastest index):

    int64_t hsl_dist[361][257][256];

As a distribution, the value of `hsl_dist[h][s][l]` indicates the number of
occurrences (frequency) of the color `(h, s, l)`.

The 360-th value of hue denotes an undefined hue (for things gray-like
colors), while the 256-th value of saturation denotes an undefined saturation
(for black or white).

## plot_hsl

This tool can be used to visualize the HSL distribution of colors.  It plots
histograms of hue, saturation, and lightness respectively.

It takes one input file of the HSL distribution in the format described in the
[rgb2hsl section](#rgb2hsl).

As an example: if you have the HSL distribution in `hsl_dist.dat`, you can
plot the histograms by running:

~~~sh
./plot_hsl hsl_dist.dat
~~~

## loadpngs

This tool takes a set of PNG images and gathers the following statistics for
them:

  - Averaged RGB value for every pixel.
  - Distribution of hue, saturation, and lightness respectively.

To build the tool, run `make loadpngs`.

As example, suppose you want to gather statistics on 3 images of size 800×600:
`input1.png`, `input2.png`, and `input3.png`.  To do this first run the
following command:

~~~sh
./loadpngs in 800 600 stats.dat input1.png input2.png input3.png
~~~

This will create a file called `stats.dat` that tracks the statistics.  You
can call this command as many times as you like with a pre-existing
`stats.dat` to add more images.  (Note that if you add the same image multiple
times, then it will be weighted higher!)

After gathering the statistics, we distill the data via:

~~~sh
./loadpngs out 800 600 stats.dat avg_rgb.png hsl_dist.dat
~~~

This creates `avg_rgb.png`, which is the shows the average RGB values as an
image, as well as `hsl_dist.dat`, which contains the HSL distribution and can
be visualized via `./plot_hsl hsl_dist.dat`.
