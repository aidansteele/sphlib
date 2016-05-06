/* $Id: cubegen.c 246 2010-06-21 21:58:05Z tp $ */
/*
 * This program generates some code for the implementation of CubeHash.
 */

#include <stdio.h>

/* obsolete
static void
permute(int x[])
{
	int i;

	for (i = 0; i < 8; i ++) {
		int t;

		t = x[i];
		x[i] = x[i + 8];
		x[i + 8] = t;
	}
	for (i = 0; i < 4; i ++) {
		int t;

		t = x[16 + (i << 2) + 0];
		x[16 + (i << 2) + 0] = x[16 + (i << 2) + 2];
		x[16 + (i << 2) + 2] = t;
		t = x[16 + (i << 2) + 1];
		x[16 + (i << 2) + 1] = x[16 + (i << 2) + 3];
		x[16 + (i << 2) + 3] = t;
	}
	for (i = 0; i < 4; i ++) {
		int t;

		t = x[i + 0];
		x[i + 0] = x[i + 4];
		x[i + 4] = t;
		t = x[i + 8];
		x[i + 8] = x[i + 12];
		x[i + 12] = t;
	}
	for (i = 0; i < 8; i ++) {
		int t;

		t = x[(i << 1) + 16];
		x[(i << 1) + 16] = x[(i << 1) + 17];
		x[(i << 1) + 17] = t;
	}
}
*/

static char *const xn[] = {
	"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
	"x8", "x9", "xa", "xb", "xc", "xd", "xe", "xf",
	"xg", "xh", "xi", "xj", "xk", "xl", "xm", "xn",
	"xo", "xp", "xq", "xr", "xs", "xt", "xu", "xv"
};

static void
print_round(int x[])
{
#define X(j)   xn[x[j]]

	int i;

	for (i = 0; i < 16; i ++) {
		printf("%s = T32(%s + %s);\n", X(i + 16), X(i), X(i + 16));
		printf("%s = ROTL32(%s, 7);\n", X(i), X(i));
	}
	for (i = 0; i < 8; i ++) {
		int t;

		t = x[i];
		x[i] = x[i + 8];
		x[i + 8] = t;
	}
	for (i = 0; i < 16; i ++) {
		printf("%s ^= %s;\n", X(i), X(i + 16));
	}
	for (i = 0; i < 4; i ++) {
		int t;

		t = x[16 + (i << 2) + 0];
		x[16 + (i << 2) + 0] = x[16 + (i << 2) + 2];
		x[16 + (i << 2) + 2] = t;
		t = x[16 + (i << 2) + 1];
		x[16 + (i << 2) + 1] = x[16 + (i << 2) + 3];
		x[16 + (i << 2) + 3] = t;
	}
	for (i = 0; i < 16; i ++) {
		printf("%s = T32(%s + %s);\n", X(i + 16), X(i), X(i + 16));
		printf("%s = ROTL32(%s, 11);\n", X(i), X(i));
	}
	for (i = 0; i < 4; i ++) {
		int t;

		t = x[i + 0];
		x[i + 0] = x[i + 4];
		x[i + 4] = t;
		t = x[i + 8];
		x[i + 8] = x[i + 12];
		x[i + 12] = t;
	}
	for (i = 0; i < 16; i ++) {
		printf("%s ^= %s;\n", X(i), X(i + 16));
	}
	for (i = 0; i < 8; i ++) {
		int t;

		t = x[(i << 1) + 16];
		x[(i << 1) + 16] = x[(i << 1) + 17];
		x[(i << 1) + 17] = t;
	}

#undef X
}

int
main(void)
{
	int x[32];
	int i;

	for (i = 0; i < 32; i ++)
		x[i] = i;
	for (i = 0; i < 2; i ++) {
		printf("/* round %d */\n", i);
		print_round(x);
	}
	return 0;
}
