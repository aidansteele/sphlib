/* $Id: fuguegen.c 177 2010-05-07 16:15:27Z tp $ */
/*
 * This code generates tables and code for Fugue.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static void
tabs(int shift)
{
	while (shift -- > 0)
		printf("\t");
}

static unsigned char Sbox[] = {
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
	0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
	0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
	0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
	0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
	0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
	0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
	0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
	0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
	0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
	0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
	0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
	0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
	0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
	0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
	0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
	0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static void
print_table(char *name, uint32_t *tab, int java)
{
	int i;

	printf("\n");
	if (java) {
		printf("\tstatic final int[] %s = {\n", name);
	} else {
		printf("static const sph_u32 %s[] = {\n", name);
	}
	if (java) {
		for (i = 0; i < 256; i ++) {
			if (i == 0) {
				printf("\t\t");
			} else if (i % 4 == 0) {
				printf(",\n\t\t");
			} else {
				printf(", ");
			}
			printf("0x%08lx", (unsigned long)tab[i]);
		}
		printf("\n\t};\n");
	} else {
		for (i = 0; i < 256; i ++) {
			if (i == 0) {
				printf("\t");
			} else if (i % 3 == 0) {
				printf(",\n\t");
			} else {
				printf(", ");
			}
			printf("SPH_C32(0x%08lx)", (unsigned long)tab[i]);
		}
		printf("\n};\n");
	}
}

static uint32_t
mul(uint32_t a, uint32_t b)
{
	uint32_t x;
	int i;

	x = 0;
	for (i = 0; i < 8; i ++) {
		if (b & (1 << i))
			x ^= a;
		a <<= 1;
		if (a & 0x100)
			a ^= 0x11b;
	}
	return x;
}

static uint32_t
rol32(uint32_t x, int n)
{
	return (x << n) | (x >> (32 - n));
}

#define ORTHO1(x0, x1)   do { \
		uint32_t t00 = (x0) & (uint32_t)0xFF00FF00; \
		uint32_t t01 = (x0) & (uint32_t)0x00FF00FF; \
		uint32_t t10 = (x1) & (uint32_t)0xFF00FF00; \
		uint32_t t11 = (x1) & (uint32_t)0x00FF00FF; \
		(x0) = t00 | (t10 >> 8); \
		(x1) = (t01 << 8) | t11; \
	} while (0)

#define ORTHO2(x0, x1)   do { \
		uint32_t t00 = (x0) & (uint32_t)0xFFFF0000; \
		uint32_t t01 = (x0) & (uint32_t)0x0000FFFF; \
		uint32_t t10 = (x1) & (uint32_t)0xFFFF0000; \
		uint32_t t11 = (x1) & (uint32_t)0x0000FFFF; \
		(x0) = t00 | (t10 >> 16); \
		(x1) = (t01 << 16) | t11; \
	} while (0)

#define ORTHO(x0, x1, x2, x3)   do { \
		ORTHO1(x0, x1); \
		ORTHO1(x2, x3); \
		ORTHO2(x0, x2); \
		ORTHO2(x1, x3); \
	} while (0)

static void
compute_smix(uint32_t *tab0, uint32_t *tab1,
	uint32_t *tab2, uint32_t *tab3, uint32_t *x)
{
	uint32_t x0 = x[0];
	uint32_t x1 = x[1];
	uint32_t x2 = x[2];
	uint32_t x3 = x[3];
	uint32_t mu00 = tab0[x0 >> 24];
	uint32_t mu01 = tab1[(x0 >> 16) & 0xFF];
	uint32_t mu02 = tab2[(x0 >>  8) & 0xFF];
	uint32_t mu03 = tab3[x0 & 0xFF];
	uint32_t mu10 = tab0[x1 >> 24];
	uint32_t mu11 = tab1[(x1 >> 16) & 0xFF];
	uint32_t mu12 = tab2[(x1 >>  8) & 0xFF];
	uint32_t mu13 = tab3[x1 & 0xFF];
	uint32_t mu20 = tab0[x2 >> 24];
	uint32_t mu21 = tab1[(x2 >> 16) & 0xFF];
	uint32_t mu22 = tab2[(x2 >>  8) & 0xFF];
	uint32_t mu23 = tab3[x2 & 0xFF];
	uint32_t mu30 = tab0[x3 >> 24];
	uint32_t mu31 = tab1[(x3 >> 16) & 0xFF];
	uint32_t mu32 = tab2[(x3 >>  8) & 0xFF];
	uint32_t mu33 = tab3[x3 & 0xFF];

	x0 = mu00 ^ mu01 ^ mu02 ^ mu03;
	x1 = mu10 ^ mu11 ^ mu12 ^ mu13;
	x2 = mu20 ^ mu21 ^ mu22 ^ mu23;
	x3 = mu30 ^ mu31 ^ mu32 ^ mu33;
	ORTHO(x0, x1, x2, x3);
	x0 ^= mu10 ^ mu20 ^ mu30;
	x1 ^= mu01 ^ mu21 ^ mu31;
	x2 ^= mu02 ^ mu12 ^ mu32;
	x3 ^= mu03 ^ mu13 ^ mu23;
	x1 = rol32(x1,  8);
	x2 = rol32(x2, 16);
	x3 = rol32(x3, 24);
	ORTHO(x0, x1, x2, x3);
	x[0] = x0;
	x[1] = x1;
	x[2] = x2;
	x[3] = x3;
}

static void
make_table(uint32_t *tab, uint32_t m0, uint32_t m1, uint32_t m2, uint32_t m3)
{
	uint32_t x;

	for (x = 0; x < 0x100; x ++) {
		uint32_t u;

		u = Sbox[x];
		tab[x] = (mul(u, m0) << 24) ^ (mul(u, m1) << 16)
			^ (mul(u, m2) << 8) ^ mul(u, m3);
	}
}

static void
make_tables(int java)
{
	uint32_t tab0[256], tab1[256], tab2[256], tab3[256];
	uint32_t N[4][16];
	int i, j;

	make_table(tab0, 1, 1, 7, 4);
	make_table(tab1, 4, 1, 1, 7);
	make_table(tab2, 7, 4, 1, 1);
	make_table(tab3, 1, 7, 4, 1);
	for (j = 0; j < 16; j ++) {
		uint32_t x[4];

		for (i = 0; i < 4; i ++)
			x[i] = 0x52525252;
		x[j / 4] -= (73 << (8 * (3 - (j % 4))));
		compute_smix(tab0, tab1, tab2, tab3, x);
		N[0][j] = x[0];
		N[1][j] = x[1];
		N[2][j] = x[2];
		N[3][j] = x[3];
	}
	print_table("mixtab0", tab0, java);
	print_table("mixtab1", tab1, java);
	print_table("mixtab2", tab2, java);
	print_table("mixtab3", tab3, java);
	printf("\n");
	printf("/*\n");
	for (i = 0; i < 16; i ++) {
		if (i != 0 && i % 4 == 0)
			printf("\n");
		for (j = 0; j < 16; j ++) {
			if (j % 4 == 0)
				printf(" ");
			printf(" %u", (unsigned)(N[i / 4][j]
				>> (8 * (3 - (i % 4)))) & 0xFF);
		}
		printf("\n");
	}
	printf("*/\n");
}

static void
printTIX2(int shift, int i, int java)
{
	if (java) {
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 10) % 30, i % 30);
		tabs(shift);
		printf("S[%2d] = w;\n", i % 30);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 8) % 30, i % 30);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 1) % 30, (i + 24) % 30);
	} else {
		tabs(shift);
		printf("TIX2(q, S%02d, S%02d, S%02d, S%02d, S%02d);\n",
			i, (i + 1) % 30, (i + 8) % 30, (i + 10) % 30,
			(i + 24) % 30);
	}
}

static void
printTIX3(int shift, int i, int java)
{
	if (java) {
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 16) % 36, i % 36);
		tabs(shift);
		printf("S[%2d] = w;\n", i % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 8) % 36, i % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 1) % 36, (i + 27) % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 4) % 36, (i + 30) % 36);
	} else {
		tabs(shift);
		printf("TIX3(q, S%02d, S%02d, S%02d, S%02d, S%02d, S%02d,"
			" S%02d);\n",
			i, (i + 1) % 36, (i + 4) % 36, (i + 8) % 36,
			(i + 16) % 36, (i + 27) % 36, (i + 30) % 36);
	}
}

static void
printTIX4(int shift, int i, int java)
{
	if (java) {
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 22) % 36, i % 36);
		tabs(shift);
		printf("S[%2d] = w;\n", i % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 8) % 36, i % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 1) % 36, (i + 24) % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 4) % 36, (i + 27) % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 7) % 36, (i + 30) % 36);
	} else {
		tabs(shift);
		printf("TIX4(q, S%02d, S%02d, S%02d, S%02d, S%02d, S%02d,"
			" S%02d, S%02d, S%02d);\n",
			i, (i + 1) % 36, (i + 4) % 36, (i + 7) % 36,
			(i + 8) % 36, (i + 22) % 36, (i + 24) % 36,
			(i + 27) % 36, (i + 30) % 36);
	}
}

static void
printCMIX30(int shift, int i, int java)
{
	if (java) {
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", i % 30, (i + 4) % 30);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 1) % 30, (i + 5) % 30);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 2) % 30, (i + 6) % 30);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 15) % 30, (i + 4) % 30);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 16) % 30, (i + 5) % 30);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 17) % 30, (i + 6) % 30);
	} else {
		tabs(shift);
		printf("CMIX30(S%02d, S%02d, S%02d, S%02d, S%02d, S%02d,"
			" S%02d, S%02d, S%02d);\n",
			i, (i + 1) % 30, (i + 2) % 30, (i + 4) % 30,
			(i + 5) % 30, (i + 6) % 30, (i + 15) % 30,
			(i + 16) % 30, (i + 17) % 30);
	}
}

static void
printCMIX36(int shift, int i, int java)
{
	if (java) {
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", i % 36, (i + 4) % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 1) % 36, (i + 5) % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 2) % 36, (i + 6) % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 18) % 36, (i + 4) % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 19) % 36, (i + 5) % 36);
		tabs(shift);
		printf("S[%2d] ^= S[%2d];\n", (i + 20) % 36, (i + 6) % 36);
	} else {
		tabs(shift);
		printf("CMIX36(S%02d, S%02d, S%02d, S%02d, S%02d, S%02d,"
			" S%02d, S%02d, S%02d);\n",
			i, (i + 1) % 36, (i + 2) % 36, (i + 4) % 36,
			(i + 5) % 36, (i + 6) % 36, (i + 18) % 36,
			(i + 19) % 36, (i + 20) % 36);
	}
}

static void
printSMIX(int shift, int i, int s, int java)
{
	if (java) {
		tabs(shift);
		printf("smix(%2d, %2d, %2d, %2d);\n",
			i % s, (i + 1) % s, (i + 2) % s, (i + 3) % s);
		/*
		int k, q;

		tabs(shift);
		printf("c0 = c1 = c2 = c3 = 0;\n");
		tabs(shift);
		printf("r0 = r1 = r2 = r3 = 0;\n");
		for (k = 0; k < 4; k ++) {
			tabs(shift);
			printf("xt = S[%2d];\n", (i + k) % s);
			for (q = 0; q < 4; q ++) {
				tabs(shift);
				printf("tmp = mixtab%d[(xt >> %2d) & 0xFF];\n",
					q, 8 * (3 - q));
				tabs(shift);
				printf("c%d ^= tmp;\n", k);
				if (k != q) {
					tabs(shift);
					printf("r%d ^= tmp;\n", q);
				}
			}
		}
		for (k = 0; k < 4; k ++) {
			for (q = 0; q < 4; q ++) {
				int sc;

				if (q == 0) {
					tabs(shift);
					printf("S[%2d] = ", (i + k) % s);
				} else {
					printf("\n");
					tabs(shift + 1);
					printf("| ");
				}
				sc = 8 * k;
				printf("((c%d ^ (r%d %s %2d)) & 0x%08lX)",
					(k + q) % 4, q,
					(q + k <= 3) ? "<< " : ">>>",
					(q + k <= 3) ? sc : 32 - sc,
					(unsigned long)0xFF000000UL >> (8 * q));
			}
			printf(";\n");
		}
		*/
	} else {
		tabs(shift);
		printf("SMIX(S%02d, S%02d, S%02d, S%02d);\n",
			i, (i + 1) % s, (i + 2) % s, (i + 3) % s);
	}
}

static void
printNEXT(int shift, int cc, int java)
{
	if (java) {
		tabs(shift);
		printf("if (num -- <= 0) {\n");
		tabs(shift + 1);
		printf("rshift = %d;\n", cc);
		tabs(shift + 1);
		printf("return;\n");
		tabs(shift);
		printf("}\n");
		tabs(shift);
		printf("w = (buf[off] << 24)\n");
		tabs(shift + 1);
		printf("| ((buf[off + 1] & 0xFF) << 16)\n");
		tabs(shift + 1);
		printf("| ((buf[off + 2] & 0xFF) << 8)\n");
		tabs(shift + 1);
		printf("| (buf[off + 3] & 0xFF);\n");
		tabs(shift);
		printf("off += 4;\n");
	} else {
		tabs(shift);
		printf("NEXT(%d);\n", cc);
	}
}

static void
print_core(int shift, int k, int s, int java)
{
	int i, cc;

	i = 0;
	cc = 0;
	printf("\n");
	printf("/* k=%d s=%d */\n", k, s);
	do {
		int z;

		tabs(shift);
		printf("case %d:\n", cc);
		if (!java) {
			tabs(shift + 1);
			printf("q = p;\n");
		}
		switch (k) {
		case 2: printTIX2(shift + 1, i, java); break;
		case 3: printTIX3(shift + 1, i, java); break;
		case 4: printTIX4(shift + 1, i, java); break;
		}
		for (z = 0; z < k; z ++) {
			i = (i + s - 3) % s;
			switch (s) {
			case 30: printCMIX30(shift + 1, i, java); break;
			case 36: printCMIX36(shift + 1, i, java); break;
			}
			printSMIX(shift + 1, i, s, java);
		}
		cc ++;
		if (i == 0)
			cc = 0;
		printNEXT(shift + 1, cc, java);
		tabs(shift + 1);
		printf("/* fall through */\n");
	} while (i != 0);
}

int
main(int argc, char *argv[])
{
	int java;

	java = argc >= 2 && strcmp(argv[1], "java") == 0;
	make_tables(java);
	print_core(2, 2, 30, java);
	print_core(2, 3, 36, java);
	print_core(2, 4, 36, java);
	if (java) {
		printf("\n");
		printf("/* SMIX */\n");
		printSMIX(2, 0, 30, 1);
	}
	return 0;
}
