all: ac32spdif dts2spdif

ac32spdif: ac32spdif.c
	cc -O2 -o ac32spdif ac32spdif.c

dts2spdif: dts2spdif.c
	cc -O2 -o dts2spdif dts2spdif.c

.PHONY: clean
clean:
	rm -f ac32spdif dts2spdif
