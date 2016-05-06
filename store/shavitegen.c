/* $Id: shavitegen.c 212 2010-06-03 00:14:38Z tp $ */
/*
 * This program generates some code for SHAvite-3.
 */

#include <stdio.h>

static void
gen_small_rk(unsigned u)
{
	if (u < 16) {
		printf("\trk%X = sph_dec32le_aligned("
			"(const unsigned char *)msg + %2d);\n", u, u * 4);
	} else if (((u - 16) % 32) < 16) {
		char *e0, *e1, *e2, *e3;

		if ((u & 3) != 0)
			return;
		printf("\tKEY_EXPAND_ELT(rk%X, rk%X, rk%X, rk%X);\n",
			(u - 16) & 15, (u - 15) & 15,
			(u - 14) & 15, (u - 13) & 15);
		e0 = e1 = e2 = e3 = "";
		if (u == 16) {
			e0 = " ^ sc->count0";
			e1 = " ^ SPH_T32(~sc->count1)";
		} else if (u == 56) {
			e1 = " ^ sc->count1";
			e2 = " ^ SPH_T32(~sc->count0)";
		} else if (u == 84) {
			e2 = " ^ sc->count1";
			e3 = " ^ SPH_T32(~sc->count0)";
		} else if (u == 124) {
			e0 = " ^ sc->count0";
			e3 = " ^ SPH_T32(~sc->count1)";
		}
		printf("\trk%X ^= rk%X%s;\n", (u + 0) & 15, (u - 4) & 15, e0);
		printf("\trk%X ^= rk%X%s;\n", (u + 1) & 15, (u - 3) & 15, e1);
		printf("\trk%X ^= rk%X%s;\n", (u + 2) & 15, (u - 2) & 15, e2);
		printf("\trk%X ^= rk%X%s;\n", (u + 3) & 15, (u - 1) & 15, e3);
	} else {
		printf("\trk%X ^= rk%X;\n", u & 15, (u - 3) & 15);
	}
}

static void
gen_small_unrolled(void)
{
	unsigned u, v;

	printf("\tsph_u32 p0, p1, p2, p3, p4, p5, p6, p7;\n");
	printf("\tsph_u32 x0, x1, x2, x3;\n");
	printf("\tsph_u32 rk0, rk1, rk2, rk3, rk4, rk5, rk6, rk7;\n");
	printf("\tsph_u32 rk8, rk9, rkA, rkB, rkC, rkD, rkE, rkF;\n");
	printf("\n");
	for (u = 0; u < 8; u ++)
		printf("\tp%X = sc->h[0x%X];\n", u, u);
	for (u = 0; u < 12; u ++) {
		unsigned x;
		unsigned po;

		printf("\t/* round %u */\n", u);
		x = 12 * u;
		po = 4 * (u & 1);
		for (v = 0; v < 4; v ++) {
			gen_small_rk(x + v);
			printf("\tx%u = p%X ^ rk%X;\n",
				v, (po + 4 + v) & 7, (x + v) & 15);
		}
		printf("\tAES_ROUND_NOKEY(x0, x1, x2, x3);\n");
		x += 4;
		for (v = 0; v < 4; v ++) {
			gen_small_rk(x + v);
			printf("\tx%u ^= rk%X;\n", v, (x + v) & 15);
		}
		printf("\tAES_ROUND_NOKEY(x0, x1, x2, x3);\n");
		x += 4;
		for (v = 0; v < 4; v ++) {
			gen_small_rk(x + v);
			printf("\tx%u ^= rk%X;\n", v, (x + v) & 15);
		}
		printf("\tAES_ROUND_NOKEY(x0, x1, x2, x3);\n");
		for (v = 0; v < 4; v ++)
			printf("\tp%X ^= x%u;\n", (po + v) & 7, v);
	}
	for (u = 0; u < 8; u ++)
		printf("\tsc->h[0x%X] ^= p%X;\n", u, u);
}

static void
gen_big_rk(unsigned u)
{
	if (u < 32) {
		printf("\trk%02X = sph_dec32le_aligned("
			"(const unsigned char *)msg + %3d);\n", u, u * 4);
	} else if (((u - 32) % 64) < 32) {
		char *e0, *e1, *e2, *e3;

		if ((u & 3) != 0)
			return;
		printf("\tKEY_EXPAND_ELT(rk%02X, rk%02X, rk%02X, rk%02X);\n",
			(u - 32) & 31, (u - 31) & 31,
			(u - 30) & 31, (u - 29) & 31);
		e0 = e1 = e2 = e3 = "";
		if (u == 32) {
			e0 = " ^ sc->count0";
			e1 = " ^ sc->count1";
			e2 = " ^ sc->count2";
			e3 = " ^ SPH_T32(~sc->count3)";
		} else if (u == 164) {
			e0 = " ^ sc->count3";
			e1 = " ^ sc->count2";
			e2 = " ^ sc->count1";
			e3 = " ^ SPH_T32(~sc->count0)";
		} else if (u == 440) {
			e0 = " ^ sc->count1";
			e1 = " ^ sc->count0";
			e2 = " ^ sc->count3";
			e3 = " ^ SPH_T32(~sc->count2)";
		} else if (u == 316) {
			e0 = " ^ sc->count2";
			e1 = " ^ sc->count3";
			e2 = " ^ sc->count0";
			e3 = " ^ SPH_T32(~sc->count1)";
		}
		printf("\trk%02X ^= rk%02X%s;\n",
			(u + 0) & 31, (u - 4) & 31, e0);
		printf("\trk%02X ^= rk%02X%s;\n",
			(u + 1) & 31, (u - 3) & 31, e1);
		printf("\trk%02X ^= rk%02X%s;\n",
			(u + 2) & 31, (u - 2) & 31, e2);
		printf("\trk%02X ^= rk%02X%s;\n",
			(u + 3) & 31, (u - 1) & 31, e3);
	} else {
		printf("\trk%02X ^= rk%02X;\n", u & 31, (u - 7) & 31);
	}
}

static void
gen_big_unrolled(void)
{
	unsigned u, v;

	printf("\tsph_u32 p0, p1, p2, p3, p4, p5, p6, p7;\n");
	printf("\tsph_u32 p8, p9, pA, pB, pC, pD, pE, pF;\n");
	printf("\tsph_u32 x0, x1, x2, x3;\n");
	printf("\tsph_u32 rk00, rk01, rk02, rk03, rk04, rk05, rk06, rk07;\n");
	printf("\tsph_u32 rk08, rk09, rk0A, rk0B, rk0C, rk0D, rk0E, rk0F;\n");
	printf("\tsph_u32 rk10, rk11, rk12, rk13, rk14, rk15, rk16, rk17;\n");
	printf("\tsph_u32 rk18, rk19, rk1A, rk1B, rk1C, rk1D, rk1E, rk1F;\n");
	printf("\n");
	for (u = 0; u < 16; u ++)
		printf("\tp%X = sc->h[0x%X];\n", u, u);
	for (u = 0; u < 14; u ++) {
		unsigned x;
		unsigned po;
		unsigned w;

		printf("\t/* round %u */\n", u);
		x = 32 * u;
		po = 4 * ((-u) & 3);
		/*
		printf("\tprintf(\"Round %u:\\n\");\n", u);
		printf("\tprintf(\"%%08X %%08X %%08X %%08X %%08X %%08X %%08X %%08X\\n\", p%X, p%X, p%X, p%X, p%X, p%X, p%X, p%X);\n",
			(po + 0x0) & 15, (po + 0x1) & 15, (po + 0x2) & 15,
			(po + 0x3) & 15, (po + 0x4) & 15, (po + 0x5) & 15,
			(po + 0x6) & 15, (po + 0x7) & 15);
		printf("\tprintf(\"%%08X %%08X %%08X %%08X %%08X %%08X %%08X %%08X\\n\", p%X, p%X, p%X, p%X, p%X, p%X, p%X, p%X);\n",
			(po + 0x8) & 15, (po + 0x9) & 15, (po + 0xA) & 15,
			(po + 0xB) & 15, (po + 0xC) & 15, (po + 0xD) & 15,
			(po + 0xE) & 15, (po + 0xF) & 15);
		printf("\tprintf(\"input to left F_4:      %%08X %%08X"
			" %%08X %%08X\\n\", p%X, p%X, p%X, p%X);\n",
			(po + 4 + 0) & 15, (po + 4 + 1) & 15,
			(po + 4 + 2) & 15, (po + 4 + 3) & 15);
		*/
		for (w = 0; w < 4; w ++) {
			for (v = 0; v < 4; v ++) {
				gen_big_rk(x + v);
				if (w == 0) {
					printf("\tx%u = p%X ^ rk%02X;\n",
						v, (po + 4 + v) & 15,
						(x + v) & 31);
				} else {
					printf("\tx%u ^= rk%02X;\n",
						v, (x + v) & 31);
				}
			}
			printf("\tAES_ROUND_NOKEY(x0, x1, x2, x3);\n");
			x += 4;
		}
		for (v = 0; v < 4; v ++)
			printf("\tp%X ^= x%u;\n", (po + v) & 15, v);
		/*
		printf("\tprintf(\"output from left F_4:   %%08X %%08X"
			" %%08X %%08X\\n\\n\", x0, x1, x2, x3);\n");
		printf("\tprintf(\"input to right F_4:     %%08X %%08X"
			" %%08X %%08X\\n\", p%X, p%X, p%X, p%X);\n",
			(po + 12 + 0) & 15, (po + 12 + 1) & 15,
			(po + 12 + 2) & 15, (po + 12 + 3) & 15);
		*/
		for (w = 0; w < 4; w ++) {
			for (v = 0; v < 4; v ++) {
				gen_big_rk(x + v);
				if (w == 0) {
					printf("\tx%u = p%X ^ rk%02X;\n",
						v, (po + 12 + v) & 15,
						(x + v) & 31);
				} else {
					printf("\tx%u ^= rk%02X;\n",
						v, (x + v) & 31);
				}
			}
			printf("\tAES_ROUND_NOKEY(x0, x1, x2, x3);\n");
			x += 4;
		}
		for (v = 0; v < 4; v ++)
			printf("\tp%X ^= x%u;\n", (po + v + 8) & 15, v);
		/*
		printf("\tprintf(\"output from right F_4:  %%08X %%08X"
			" %%08X %%08X\\n\\n\", x0, x1, x2, x3);\n");
		*/
	}
	/*
	 * 14 is not a multiple of 4, so the p* words are virtually rotated.
	 */
	for (u = 0; u < 16; u ++)
		printf("\tsc->h[0x%X] ^= p%X;\n", u, (u + 8) & 15);
}

int
main(void)
{
	printf("/* Unrolled C_256 */\n");
	gen_small_unrolled();
	printf("\n");
	printf("/* Unrolled C_512 */\n");
	gen_big_unrolled();
	return 0;
}
