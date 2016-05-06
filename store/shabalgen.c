/* $Id: shabalgen.c 116 2009-03-12 19:38:57Z tp $ */
/*
 * This program generates part of the code in shabal_opt.c. It is
 * configured at compilation time by setting the SHABAL_PARAM_P and
 * SHABAL_PARAM_R. If non-default values are set for those macros, then
 * the same values MUST be used when compiling the resulting
 * shabal_opt.c file. The generated code includes a failsafe detection
 * system which checks exactly for that property.
 *
 * The generated code is written out on standard output. That code
 * chunk must be pasted in shabal_opt.c between the "BEGIN" and "END"
 * comments.
 *
 * (c) 2008 SAPHIR project. This software is provided 'as-is', without
 * any express or implied warranty. In no event will the authors be held
 * liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to no restriction.
 *
 * Technical remarks and questions can be addressed to:
 * <thomas.pornin@cryptolog.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SHABAL_PARAM_P
#define SHABAL_PARAM_P  3
#endif
#ifndef SHABAL_PARAM_R
#define SHABAL_PARAM_R  12
#endif

#define sM   16
#define nP    SHABAL_PARAM_P
#define nR    SHABAL_PARAM_R

#define O1   13
#define O2    9
#define O3    6

/*
 * Note: in this code, we compute part of a Shabal permutation, to
 * generate the precomputed IV. We use the "unsigned long" type which
 * is at least 32-bit wide on all systems. On 64-bit architectures,
 * this type may be quite wider than 32 bits, hence the truncation
 * to 32 bits by the T32() macro is important.
 */

typedef struct {
	unsigned long A[nR];
	unsigned long B[sM];
	unsigned long C[sM];
} shabal_state;

#define T32(x)   ((x) & 0xFFFFFFFFUL)

/*
 * This function computes and processes the prefix (two initial blocks).
 */
static void
process_prefix(shabal_state *state, int hashbitlen)
{
#define xA     (state->A[(i + sM * j) % nR])
#define xAm1   (state->A[(i + sM * j + (nR - 1)) % nR])
#define U(x)   T32(3UL * (x))
#define V(x)   T32(5UL * (x))

	int i, j, k;
	unsigned long m[sM];

	/*
	 * Initial state.
	 *
	 * We have two prefix blocks to process. W is -1 for the first
	 * block, 0 for the second. W is XORed with A[0] and A[1], so
	 * the net effect is that A[0] and A[1] are bit-flipped before
	 * applying the permutation for the first block (XOR with
	 * all-ones), and left unchanged for the second block (XOR with
	 * all-zeroes). We merely set A[0] and A[1] manually to all-ones.
	 * The other A[i] are initally zero. We may now ignore W for the
	 * rest of the IV computation.
	 */
	state->A[0] = 0xFFFFFFFFUL;
	state->A[1] = 0xFFFFFFFFUL;
	for (i = 2; i < nR; i ++)
		state->A[i] = 0;
	for (i = 0; i < sM; i ++) {
		state->B[i] = 0;
		state->C[i] = 0;
	}

	for (k = 0; k < 2; k ++) {
		/*
		 * We compute the message blocks, and add them to B.
		 */
		for (i = 0; i < sM; i ++) {
			m[i] = hashbitlen + (k * sM) + i;
			state->B[i] = T32(state->B[i] + m[i]);
		}

		/*
		 * The permutation.
		 */
		for (i = 0; i < sM; i ++) {
			unsigned long t;

			t = state->B[i];
			state->B[i] = T32(t << 17) | (t >> 15);
		}
		for (j = 0; j < nP; j ++) {
			for (i = 0; i < sM; i ++) {
				unsigned long tB;

				xA = U(xA ^ V(T32(xAm1 << 15) | (xAm1 >> 17))
						^ state->C[(8 + sM - i) % sM])
					^ state->B[(i + O1) % sM]
					^ (state->B[(i + O2) % sM]
						& ~state->B[(i + O3) % sM])
					^ m[i];
				tB = state->B[i];
				state->B[i] = T32(((tB << 1) | (tB >> 31))
					^ ~xA);
			}
		}

		for (j = 0; j < nR; j ++) {
			state->A[j] = T32(state->A[j]
				+ state->C[(1 + 6 + nR + j) % sM]
				+ state->C[(1 + 6 + 2 * nR + j) % sM]
				+ state->C[(1 + 6 + 3 * nR + j) % sM]);
		}

		/*
		 * We subtract the block from the C buffer, and
		 * then swap B and C.
		 */
		for (i = 0; i < sM; i ++) {
			unsigned long t;

			t = state->B[i];
			state->B[i] = T32(state->C[i] - m[i]);
			state->C[i] = t;
		}
	}

#undef xA
#undef xAm1
#undef U
#undef V
}

static void
make_array(char *name, unsigned long *d, int len)
{
	int i;

	printf("\n");
	printf("static const u32 %s[] = {\n", name);
	for (i = 0; i < len; i ++) {
		if (i == 0) {
			printf("\t");
		} else if (i % 4 == 0) {
			printf(",\n\t");
		} else {
			printf(", ");
		}
		printf("C32(0x%08lX)", d[i]);
	}
	printf("\n};\n");
}

static void
make_iv(int hashbitlen)
{
	shabal_state st;
	char name[20];

	process_prefix(&st, hashbitlen);
	sprintf(name, "A_init_%d", hashbitlen);
	make_array(name, st.A, nR);
	sprintf(name, "B_init_%d", hashbitlen);
	make_array(name, st.B, sM);
	sprintf(name, "C_init_%d", hashbitlen);
	make_array(name, st.C, sM);
}

/*
 * The generated code is printed on the standard output.
 */
int
main(void)
{
	int i, j;

	printf("\n");

	printf("#if SHABAL_PARAM_P != %d || SHABAL_PARAM_R != %d\n", nP, nR);
	printf("#error Parameters changed; code below is not valid\n");
	printf("#endif\n");

	printf("\n");

	printf("#define DECL_STATE   \\\n");
	for (i = 0; i < nR; i ++) {
		if (i == 0) {
			printf("\tu32 ");
		} else if (i % 8 == 0) {
			printf(", \\\n\t    ");
		} else {
			printf(", ");
		}
		printf("A%02X", (unsigned)i);
	}
	printf("; \\\n");
	printf("\tu32 B0, B1, B2, B3, B4, B5, B6, B7, \\\n");
	printf("\t    B8, B9, BA, BB, BC, BD, BE, BF; \\\n");
	printf("\tu32 C0, C1, C2, C3, C4, C5, C6, C7, \\\n");
	printf("\t    C8, C9, CA, CB, CC, CD, CE, CF; \\\n");
	printf("\tu32 M0, M1, M2, M3, M4, M5, M6, M7, \\\n");
	printf("\t    M8, M9, MA, MB, MC, MD, ME, MF; \\\n");
	printf("\tu32 Wlow, Whigh;\n");

	printf("\n");

	printf("#define READ_STATE(state)   do { \\\n");
	for (i = 0; i < nR; i ++)
		printf("\t\tA%02X = (state)->A[%d]; \\\n", i, i);
	for (i = 0; i < sM; i ++)
		printf("\t\tB%X = (state)->B[%d]; \\\n", i, i);
	for (i = 0; i < sM; i ++)
		printf("\t\tC%X = (state)->C[%d]; \\\n", i, i);
	printf("\t\tWlow = (state)->Wlow; \\\n");
	printf("\t\tWhigh = (state)->Whigh; \\\n");
	printf("\t} while (0)\n");

	printf("\n");

	printf("#define WRITE_STATE(state)   do { \\\n");
	for (i = 0; i < nR; i ++)
		printf("\t\t(state)->A[%d] = A%02X; \\\n", i, i);
	for (i = 0; i < sM; i ++)
		printf("\t\t(state)->B[%d] = B%X; \\\n", i, i);
	for (i = 0; i < sM; i ++)
		printf("\t\t(state)->C[%d] = C%X; \\\n", i, i);
	printf("\t\t(state)->Wlow = Wlow; \\\n");
	printf("\t\t(state)->Whigh = Whigh; \\\n");
	printf("\t} while (0)\n");

	printf("\n");

	printf("#define DECODE_BLOCK   do { \\\n");
	for (i = 0; i < sM; i ++)
		printf("\t\tM%X = dec32le_aligned(buffer + %d); \\\n",
			i, 4 * i);
	printf("\t} while (0)\n");

	printf("\n");

	printf("#define INPUT_BLOCK_ADD   do { \\\n");
	for (i = 0; i < sM; i ++)
		printf("\t\tB%X = T32(B%X + M%X); \\\n", i, i, i);
	printf("\t} while (0)\n");

	printf("\n");

	printf("#define INPUT_BLOCK_SUB   do { \\\n");
	for (i = 0; i < sM; i ++)
		printf("\t\tC%X = T32(C%X - M%X); \\\n", i, i, i);
	printf("\t} while (0)\n");

	printf("\n");

	printf("#define XOR_W   do { \\\n");
	printf("\t\tA00 ^= Wlow; \\\n");
	printf("\t\tA01 ^= Whigh; \\\n");
	printf("\t} while (0)\n");

	printf("\n");

	printf("#define SWAP(v1, v2)   do { \\\n");
	printf("\t\tu32 tmp = (v1); \\\n");
	printf("\t\t(v1) = (v2); \\\n");
	printf("\t\t(v2) = tmp; \\\n");
	printf("\t} while (0)\n");

	printf("\n");

	printf("#define SWAP_BC   do { \\\n");
	for (i = 0; i < sM; i ++)
		printf("\t\tSWAP(B%X, C%X); \\\n", i, i);
	printf("\t} while (0)\n");

	printf("\n");

	printf("#define PERM_ELT(xa0, xa1, xb0, xb1, xb2, xb3, xc, xm)"
		"   do { \\\n");
	printf("\t\txa0 = T32((xa0 \\\n");
	printf("\t\t\t^ (((xa1 << 15) | (xa1 >> 17)) * 5U) \\\n");
	printf("\t\t\t^ xc) * 3U) \\\n");
	printf("\t\t\t^ xb1 ^ (xb2 & ~xb3) ^ xm; \\\n");
	printf("\t\txb0 = T32(~(((xb0 << 1) | (xb0 >> 31)) ^ xa0)); \\\n");
	printf("\t} while (0)\n");

	for (j = 0; j < nP; j ++) {
		printf("\n");
		printf("#define PERM_STEP_%d   do { \\\n", j);
		for (i = 0; i < sM; i ++)
			printf("\t\tPERM_ELT(A%02X, A%02X, B%X,"
				" B%X, B%X, B%X, C%X, M%X); \\\n",
				(i + sM * j) % nR,
				(i + sM * j + (nR - 1)) % nR,
				i, (i + O1) % sM, (i + O2) % sM, (i + O3) % sM,
				(8 + sM - i) % sM, i);
		printf("\t} while (0)\n");
	}

	printf("\n");

	printf("#define APPLY_P   do { \\\n");
	for (i = 0; i < sM; i ++)
		printf("\t\tB%X = T32(B%X << 17) | (B%X >> 15); \\\n",
			i, i, i);
	for (j = 0; j < nP; j ++)
		printf("\t\tPERM_STEP_%d; \\\n", j);

	/*
	 * It seems easier for the compiler to optimize code when we
	 * express the 36 additions as so many individual expressions.
	 */
	for (j = 0; j < (3 * nR); j ++)
		printf("\t\tA%02X = T32(A%02X + C%X); \\\n",
			(4 * nR - 1 - j) % nR, (4 * nR - 1 - j) % nR,
			(3 * nR * sM + 6 - j) % sM);

	printf("\t} while (0)\n");

	printf("\n");

	printf("#define INCR_W   do { \\\n");
	printf("\t\tif ((Wlow = T32(Wlow + 1)) == 0) \\\n");
	printf("\t\t\tWhigh = T32(Whigh + 1); \\\n");
	printf("\t} while (0)\n");

	make_iv(192);
	make_iv(224);
	make_iv(256);
	make_iv(384);
	make_iv(512);

	printf("\n");

	return 0;
}
