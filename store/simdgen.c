/* $Id: simdgen.c 177 2010-05-07 16:15:27Z tp $ */
/*
 * This code generates tables for SIMD.
 */

#include <stdio.h>

static int
pow257(int x, unsigned n)
{
	int r;

	r = 1;
	for (;;) {
		if (n & 1)
			r = (int)(((long)r * x) % 257);
		n >>= 1;
		if (n == 0)
			return r;
		x = (int)(((long)x * x) % 257);
	}
}

static int
mkoff_s_n(int i)
{
	return pow257(139, 127 * i);
}

static int
mkoff_s_f(int i)
{
	return (pow257(139, 127 * i) + pow257(139, 125 * i)) % 257;
}

static int
mkoff_b_n(int i)
{
	return pow257(41, 255 * i);
}

static int
mkoff_b_f(int i)
{
	return (pow257(41, 255 * i) + pow257(41, 253 * i)) % 257;
}

static void
print_tab(char *name, int max, int (*fun)(int))
{
	int i;

	printf("\n");
	printf("static const unsigned short %s[] = {\n", name);
	for (i = 0; i < max; i ++) {
		if (i == 0) {
			printf("\t");
		} else if (i % 12 == 0) {
			printf(",\n\t");
		} else {
			printf(", ");
		}
		printf("%3d", fun(i));
	}
	printf("\n};\n");
}

int
main(void)
{
	print_tab("yoff_s_n", 128, mkoff_s_n);
	print_tab("yoff_s_f", 128, mkoff_s_f);
	print_tab("yoff_b_n", 256, mkoff_b_n);
	print_tab("yoff_b_f", 256, mkoff_b_f);
	return 0;
}
