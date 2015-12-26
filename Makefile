CFLAGS=-Wall -O2 -g

all: loadpngs rgb2hsl

run: plot_hsl hsl.dat
	./plot_hsl hsl.dat

clean:
	rm -f hsl.dat rgb.dat rgb2hsl

rgb.dat: npy2dat rgb.npy
	./npy2dat rgb.npy $@

hsl.dat: rgb2hsl rgb.dat
	./rgb2hsl rgb.dat $@

loadpngs.o: color.h util.h

rgb2hsl.o: color.h util.h

util.o: util.h

loadpngs: loadpngs.o color.o util.o
	$(CC) $(CFLAGS) -o $@ loadpngs.o color.o util.o -lm -lpng

rgb2hsl: rgb2hsl.o color.o util.o
	$(CC) $(CFLAGS) -o $@ rgb2hsl.o color.o util.o -lm

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY: all clean run
.SUFFIXES:
.SUFFIXES: .c .o
