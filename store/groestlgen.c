/* $Id: groestlgen.c 199 2010-05-27 20:17:33Z tp $ */
/*
 * This code generates tables and code for Groestl.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
print_table(char *name, uint64_t *tab, int java)
{
	int i;

	printf("\n");
	if (java) {
		printf("\tstatic final long[] %s = {\n", name);
	} else {
		printf("static const sph_u64 %s[] = {\n", name);
	}
	if (java) {
		for (i = 0; i < 256; i ++) {
			if (i == 0) {
				printf("\t\t");
			} else if (i % 2 == 0) {
				printf(",\n\t\t");
			} else {
				printf(", ");
			}
			printf("0x%016llxL", (unsigned long long)tab[i]);
		}
		printf("\n\t};\n");
	} else {
		for (i = 0; i < 256; i ++) {
			if (i == 0) {
				printf("\t");
			} else if (i % 2 == 0) {
				printf(",\n\t");
			} else {
				printf(", ");
			}
			printf("C64e(0x%016llx)", (unsigned long long)tab[i]);
		}
		printf("\n};\n");
	}
}

static void
print_table_32(char *name, uint64_t *tab, int java)
{
	int i;

	if (java) {
		printf("\n");
		printf("\tstatic final int[] %sup = {\n", name);
		for (i = 0; i < 256; i ++) {
			if (i == 0) {
				printf("\t\t");
			} else if (i % 3 == 0) {
				printf(",\n\t\t");
			} else {
				printf(", ");
			}
			printf("0x%08lx", (unsigned long)(tab[i] >> 32));
		}
		printf("\n\t};\n");
		printf("\n");
		printf("\tstatic final int[] %sdn = {\n", name);
		for (i = 0; i < 256; i ++) {
			if (i == 0) {
				printf("\t\t");
			} else if (i % 3 == 0) {
				printf(",\n\t\t");
			} else {
				printf(", ");
			}
			printf("0x%08lx", (unsigned long)(uint32_t)tab[i]);
		}
		printf("\n\t};\n");
	} else {
		printf("\n");
		printf("static const sph_u32 %sup[] = {\n", name);
		for (i = 0; i < 256; i ++) {
			if (i == 0) {
				printf("\t");
			} else if (i % 4 == 0) {
				printf(",\n\t");
			} else {
				printf(", ");
			}
			printf("C32e(0x%08lx)", (unsigned long)(tab[i] >> 32));
		}
		printf("\n};\n");
		printf("\n");
		printf("static const sph_u32 %sdn[] = {\n", name);
		for (i = 0; i < 256; i ++) {
			if (i == 0) {
				printf("\t");
			} else if (i % 4 == 0) {
				printf(",\n\t");
			} else {
				printf(", ");
			}
			printf("C32e(0x%08lx)",
				(unsigned long)(uint32_t)tab[i]);
		}
		printf("\n};\n");
	}
}

static unsigned
mul(unsigned a, unsigned b)
{
	unsigned x;
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

static void
make_table(uint64_t *tab, unsigned bcol)
{
	static unsigned bcol0[] = { 2, 7, 5, 3, 5, 4, 3, 2 };

	unsigned x;
	unsigned bb[8];

	for (x = 0; x < 8; x ++)
		bb[x] = bcol0[(x + 8 - bcol) & 7];
	for (x = 0; x < 0x100; x ++) {
		unsigned u;

		u = Sbox[x];
		tab[x] = ((uint64_t)mul(u, bb[0]) << 56)
			^ ((uint64_t)mul(u, bb[1]) << 48)
			^ ((uint64_t)mul(u, bb[2]) << 40)
			^ ((uint64_t)mul(u, bb[3]) << 32)
			^ ((uint64_t)mul(u, bb[4]) << 24)
			^ ((uint64_t)mul(u, bb[5]) << 16)
			^ ((uint64_t)mul(u, bb[6]) << 8)
			^ ((uint64_t)mul(u, bb[7]) << 0);
	}
}

static void
make_tables(int java)
{
	int i;

	for (i = 0; i < 8; i ++) {
		uint64_t tab[256];
		char name[20];

		make_table(tab, i);
		sprintf(name, "T%d", i);
		print_table(name, tab, java);
	}
}

static void
make_tables_32(int java)
{
	int i;

	for (i = 0; i < 4; i ++) {
		uint64_t tab1[256], tab2[256];
		int j;

		make_table(tab1, i);
		make_table(tab2, i + 4);
		for (j = 0; j < 256; j ++) {
			uint64_t v;

			v = tab1[j];
			if (((v << 32) | (v >> 32)) != tab2[j])
				abort();
		}
	}

	for (i = 0; i < 4; i ++) {
		uint64_t tab[256];
		char name[20];

		make_table(tab, i);
		sprintf(name, "T%d", i);
		print_table_32(name, tab, java);
	}
}

int
main(int argc, char *argv[])
{
	int java;

	java = argc >= 2 && strcmp(argv[1], "java") == 0;
	make_tables(java);
	make_tables_32(java);
	return 0;
}
