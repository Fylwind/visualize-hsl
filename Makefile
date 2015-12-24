CFLAGS=-Wall -O2 -g

all: run

run: plot_hsl hsl.dat
	./plot_hsl hsl.dat

clean:
	rm -f hsl.dat rgb.dat rgb2hsl

rgb.dat: npy2dat rgb.npy
	./npy2dat rgb.npy $@

hsl.dat: rgb2hsl rgb.dat
	./rgb2hsl rgb.dat $@

rgb2hsl: rgb2hsl.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ rgb2hsl.c

.PHONY: all clean run
