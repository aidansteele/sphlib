/* $Id: unroll.c 35 2007-01-14 16:22:57Z tp $ */
/*
 * This code is used to generate unrolled C code for some hash functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if UINT_MAX < 0xFFFFFFFF
#error This code requires 32-bit ints (or more)
#endif

/*
 * Constants for SHA-1.
 */
static const unsigned K1[4] = {
	0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6
};

/*
 * Constants for SHA-224 / SHA-256.
 */
static const unsigned K256[64] = {
	0x428A2F98U, 0x71374491U, 0xB5C0FBCFU,
	0xE9B5DBA5U, 0x3956C25BU, 0x59F111F1U,
	0x923F82A4U, 0xAB1C5ED5U, 0xD807AA98U,
	0x12835B01U, 0x243185BEU, 0x550C7DC3U,
	0x72BE5D74U, 0x80DEB1FEU, 0x9BDC06A7U,
	0xC19BF174U, 0xE49B69C1U, 0xEFBE4786U,
	0x0FC19DC6U, 0x240CA1CCU, 0x2DE92C6FU,
	0x4A7484AAU, 0x5CB0A9DCU, 0x76F988DAU,
	0x983E5152U, 0xA831C66DU, 0xB00327C8U,
	0xBF597FC7U, 0xC6E00BF3U, 0xD5A79147U,
	0x06CA6351U, 0x14292967U, 0x27B70A85U,
	0x2E1B2138U, 0x4D2C6DFCU, 0x53380D13U,
	0x650A7354U, 0x766A0ABBU, 0x81C2C92EU,
	0x92722C85U, 0xA2BFE8A1U, 0xA81A664BU,
	0xC24B8B70U, 0xC76C51A3U, 0xD192E819U,
	0xD6990624U, 0xF40E3585U, 0x106AA070U,
	0x19A4C116U, 0x1E376C08U, 0x2748774CU,
	0x34B0BCB5U, 0x391C0CB3U, 0x4ED8AA4AU,
	0x5B9CCA4FU, 0x682E6FF3U, 0x748F82EEU,
	0x78A5636FU, 0x84C87814U, 0x8CC70208U,
	0x90BEFFFAU, 0xA4506CEBU, 0xBEF9A3F7U,
	0xC67178F2U
};

/*
 * Constant for SHA-384 / SHA-512. This requires the "unsigned long long"
 * type. Note that we have no reliable test for this type (some compilers,
 * e.g. gcc in pre-C99 mode, provide this type but not ULLONG_MAX).
 */
static const unsigned long long K512[80] = {
	0x428A2F98D728AE22ULL, 0x7137449123EF65CDULL,
	0xB5C0FBCFEC4D3B2FULL, 0xE9B5DBA58189DBBCULL,
	0x3956C25BF348B538ULL, 0x59F111F1B605D019ULL,
	0x923F82A4AF194F9BULL, 0xAB1C5ED5DA6D8118ULL,
	0xD807AA98A3030242ULL, 0x12835B0145706FBEULL,
	0x243185BE4EE4B28CULL, 0x550C7DC3D5FFB4E2ULL,
	0x72BE5D74F27B896FULL, 0x80DEB1FE3B1696B1ULL,
	0x9BDC06A725C71235ULL, 0xC19BF174CF692694ULL,
	0xE49B69C19EF14AD2ULL, 0xEFBE4786384F25E3ULL,
	0x0FC19DC68B8CD5B5ULL, 0x240CA1CC77AC9C65ULL,
	0x2DE92C6F592B0275ULL, 0x4A7484AA6EA6E483ULL,
	0x5CB0A9DCBD41FBD4ULL, 0x76F988DA831153B5ULL,
	0x983E5152EE66DFABULL, 0xA831C66D2DB43210ULL,
	0xB00327C898FB213FULL, 0xBF597FC7BEEF0EE4ULL,
	0xC6E00BF33DA88FC2ULL, 0xD5A79147930AA725ULL,
	0x06CA6351E003826FULL, 0x142929670A0E6E70ULL,
	0x27B70A8546D22FFCULL, 0x2E1B21385C26C926ULL,
	0x4D2C6DFC5AC42AEDULL, 0x53380D139D95B3DFULL,
	0x650A73548BAF63DEULL, 0x766A0ABB3C77B2A8ULL,
	0x81C2C92E47EDAEE6ULL, 0x92722C851482353BULL,
	0xA2BFE8A14CF10364ULL, 0xA81A664BBC423001ULL,
	0xC24B8B70D0F89791ULL, 0xC76C51A30654BE30ULL,
	0xD192E819D6EF5218ULL, 0xD69906245565A910ULL,
	0xF40E35855771202AULL, 0x106AA07032BBD1B8ULL,
	0x19A4C116B8D2D0C8ULL, 0x1E376C085141AB53ULL,
	0x2748774CDF8EEB99ULL, 0x34B0BCB5E19B48A8ULL,
	0x391C0CB3C5C95A63ULL, 0x4ED8AA4AE3418ACBULL,
	0x5B9CCA4F7763E373ULL, 0x682E6FF3D6B2B8A3ULL,
	0x748F82EE5DEFB2FCULL, 0x78A5636F43172F60ULL,
	0x84C87814A1F0AB72ULL, 0x8CC702081A6439ECULL,
	0x90BEFFFA23631E28ULL, 0xA4506CEBDE82BDE9ULL,
	0xBEF9A3F7B2C67915ULL, 0xC67178F2E372532BULL,
	0xCA273ECEEA26619CULL, 0xD186B8C721C0C207ULL,
	0xEADA7DD6CDE0EB1EULL, 0xF57D4F7FEE6ED178ULL,
	0x06F067AA72176FBAULL, 0x0A637DC5A2C898A6ULL,
	0x113F9804BEF90DAEULL, 0x1B710B35131C471BULL,
	0x28DB77F523047D84ULL, 0x32CAAB7B40C72493ULL,
	0x3C9EBE0A15C9BEBCULL, 0x431D67C49C100D4CULL,
	0x4CC5D4BECB3E42B6ULL, 0x597F299CFC657E2AULL,
	0x5FCB6FAB3AD6FAECULL, 0x6C44198C4A475817ULL
};

static void
ptab(int tab_level)
{
	while (tab_level -- > 0)
		putchar('\t');
}

static void
peol(int macro)
{
	puts(macro ? " \\" : "");
}

static void
gensha1(int tab_level, int macro)
{
	int i;
	char *role_A, *role_B, *role_C, *role_D, *role_E;

	role_A = "A";
	role_B = "B";
	role_C = "C";
	role_D = "D";
	role_E = "E";
	for (i = 0; i < 80; i ++) {
		char *tmp;

		ptab(tab_level);
		printf("W%02d = ", i & 15);
		if (i < 16)
			printf("in(%d);", 4 * i);
		else
			printf("ROTL(W%02d ^ W%02d ^ W%02d ^ W%02d, 1);",
				(i - 3) & 15, (i - 8) & 15,
				(i - 14) & 15, (i - 16) & 15);
		peol(macro);
		ptab(tab_level);
		printf("%s = SPH_T32(ROTL(%s, 5) + %c(%s, %s, %s) + %s",
			role_E, role_A, 'F' + (i / 20),
			role_B, role_C, role_D, role_E);
		peol(macro);
		ptab(tab_level + 1);
		printf("+ W%02d + SPH_C32(0x%08X));", i & 15, K1[i / 20]);
		peol(macro);
		ptab(tab_level);
		printf("%s = ROTL(%s, 30);", role_B, role_B);
		peol(macro);
		tmp = role_E;
		role_E = role_D;
		role_D = role_C;
		role_C = role_B;
		role_B = role_A;
		role_A = tmp;
	}
}

static void
gensha2(int tab_level, int macro)
{
	int i;
	char *role_A, *role_B, *role_C, *role_D;
	char *role_E, *role_F, *role_G, *role_H;

	role_A = "A";
	role_B = "B";
	role_C = "C";
	role_D = "D";
	role_E = "E";
	role_F = "F";
	role_G = "G";
	role_H = "H";
	for (i = 0; i < 64; i ++) {
		char *tmp;

		ptab(tab_level);
		printf("W%02d = ", i & 15);
		if (i < 16)
			printf("in(%d);", i);
		else
			printf("SPH_T32(SSG2_1(W%02d) + W%02d"
				" + SSG2_0(W%02d) + W%02d);",
				(i - 2) & 15, (i - 7) & 15,
				(i - 15) & 15, (i - 16) & 15);
		peol(macro);
		ptab(tab_level);
		printf("T1 = SPH_T32(%s + BSG2_1(%s) + CH(%s, %s, %s)",
			role_H, role_E, role_E, role_F, role_G);
		peol(macro);
		ptab(tab_level + 1);
		printf("+ SPH_C32(0x%08X) + W%02d);", K256[i], (i & 15));
		peol(macro);
		ptab(tab_level);
		printf("T2 = SPH_T32(BSG2_0(%s) + MAJ(%s, %s, %s));",
			role_A, role_A, role_B, role_C);
		peol(macro);
		ptab(tab_level);
		printf("%s = SPH_T32(%s + T1);", role_D, role_D);
		peol(macro);
		ptab(tab_level);
		printf("%s = SPH_T32(T1 + T2);", role_H);
		peol(macro);
		tmp = role_H;
		role_H = role_G;
		role_G = role_F;
		role_F = role_E;
		role_E = role_D;
		role_D = role_C;
		role_C = role_B;
		role_B = role_A;
		role_A = tmp;
	}
}

static void
gensha3(int tab_level, int macro)
{
	int i;
	char *role_A, *role_B, *role_C, *role_D;
	char *role_E, *role_F, *role_G, *role_H;

	role_A = "A";
	role_B = "B";
	role_C = "C";
	role_D = "D";
	role_E = "E";
	role_F = "F";
	role_G = "G";
	role_H = "H";
	for (i = 0; i < 80; i ++) {
		char *tmp;

		ptab(tab_level);
		printf("W%02d = ", i & 15);
		if (i < 16)
			printf("in(%d);", i);
		else
			printf("SPH_T64(SSG5_1(W%02d) + W%02d"
				" + SSG5_0(W%02d) + W%02d);",
				(i - 2) & 15, (i - 7) & 15,
				(i - 15) & 15, (i - 16) & 15);
		peol(macro);
		ptab(tab_level);
		printf("T1 = SPH_T64(%s + BSG5_1(%s) + CH(%s, %s, %s)",
			role_H, role_E, role_E, role_F, role_G);
		peol(macro);
		ptab(tab_level + 1);
		printf("+ SPH_C64(0x%08llX) + W%02d);", K512[i], (i & 15));
		peol(macro);
		ptab(tab_level);
		printf("T2 = SPH_T64(BSG5_0(%s) + MAJ(%s, %s, %s));",
			role_A, role_A, role_B, role_C);
		peol(macro);
		ptab(tab_level);
		printf("%s = SPH_T64(%s + T1);", role_D, role_D);
		peol(macro);
		ptab(tab_level);
		printf("%s = SPH_T64(T1 + T2);", role_H);
		peol(macro);
		tmp = role_H;
		role_H = role_G;
		role_G = role_F;
		role_F = role_E;
		role_E = role_D;
		role_D = role_C;
		role_C = role_B;
		role_B = role_A;
		role_A = tmp;
	}
}

static struct {
	char *name;
	char *comment;
	void (*genf)(int, int);
} algos[] = {
	{ "sha1", "SHA-1",                  &gensha1 },
	{ "sha2", "SHA-224 and SHA-256",    &gensha2 },
	{ "sha3", "SHA-384 and SHA-512",    &gensha3 },
	{ NULL, NULL, 0 }
};

static void
usage(void)
{
	size_t u;

	fprintf(stderr, "usage: unroll [ -m ] [ -# ] algo\n");
	fprintf(stderr, "\"algo\" is one in:\n");
	for (u = 0; algos[u].name != NULL; u ++) {
		char *name, *comment;
		char buf[8];

		name = algos[u].name;
		comment = algos[u].comment;
		memset(buf, ' ', sizeof buf);
		buf[(sizeof buf) - 1] = 0;
		memcpy(buf, name, strlen(name));
		printf("%sfor %s\n", buf, name);
	}
	fprintf(stderr, "  -m   generate for a macro (adds final '\\')\n");
	fprintf(stderr, "  -#   indent by '#' tabs (one digit)\n");
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int i, macro, tab_level;
	void (*genf)(int, int);

	genf = 0;
	macro = 0;
	tab_level = -1;
	for (i = 1; i < argc; i ++) {
		if (argv[i][0] == '-') {
			int c;

			c = argv[i][1];
			if (c == 'm') {
				macro = 1;
			} else if (c >= '0' && c <= '9') {
				int tl;

				tl = c - '0';
				if (tab_level >= 0 && tab_level != tl)
					usage();
				tab_level = tl;
			} else {
				fprintf(stderr,
					"unknown option: '%s'\n", argv[i]);
				usage();
			}
		} else {
			size_t u;

			if (genf != 0)
				usage();
			for (u = 0; algos[u].name != NULL; u ++) {
				if (strcmp(argv[i], algos[u].name) == 0) {
					genf = algos[u].genf;
					break;
				}
			}
			if (genf == 0) {
				fprintf(stderr,
					"unknown algorithm name: '%s'\n",
					argv[i]);
				usage();
			}
		}
	}
	if (genf == 0)
		usage();
	if (tab_level < 0)
		tab_level = 1;
	genf(tab_level, macro);
	return 0;
}
