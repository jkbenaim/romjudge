/* jrra 2023 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
//#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "endian.h"
#include "err.h"
#include "grader.h"
#include "ipltxt.h"
#include "sha1.h"

struct {
	char *name;
	int32_t epOffset;
	uint32_t seed;
	const uint8_t *txt;
	uint8_t signature[20];
	bool end;
} cics[] = {
	[IPL_NONE] = {
		.name = "unrecognized",
		.signature = {0},
	},
	[IPL_6101] = {
		.name = "6101",
		.epOffset = 0,
		.seed = 0x3f * 0x5d588b65u + 1,
		.txt = ipl_6101,
		.signature = {0xea, 0xad, 0xcb, 0x8c, 0xca, 0x9c, 0x6b, 0xa1,
			      0x44, 0x5f, 0x98, 0xf1, 0x72, 0x7b, 0xf4, 0xad,
			      0xba, 0xbb, 0x88, 0xb2},
	},
	[IPL_6102] = {
		.name = "6102 or 7101",
		.epOffset = 0,
		.seed = 0x3f * 0x5d588b65u + 1,
		.txt = ipl_6102,
		.signature = {0xb2, 0xaf, 0xae, 0x24, 0x6e, 0x1d, 0xab, 0x74,
			      0x6b, 0xfb, 0x28, 0xcb, 0x34, 0x6e, 0x29, 0x11,
			      0x96, 0x5e, 0xef, 0xa1},
	},
	[IPL_6103] = {
		.name = "6103 or 7103",
		.epOffset = -0x100000,
		.seed = 0x78 * 0x6c078965u + 1,
		.txt = ipl_6103,
		.signature = {0x3f, 0x73, 0x47, 0xaa, 0x04, 0x26, 0xee, 0x97,
			      0xd9, 0x67, 0x21, 0xb0, 0x9b, 0x91, 0x8d, 0x4c,
			      0x9c, 0xdd, 0xb6, 0x9b},
	},
	[IPL_6105] = {
		.name = "6105 or 7105",
		.epOffset = 0,
		.seed = 0x91 * 0x5d588b65u + 1,
		.txt = ipl_6105,
		.signature = {0x41, 0x59, 0x26, 0x90, 0x55, 0xe8, 0xa5, 0xbe,
			      0x2e, 0x5c, 0x8e, 0x3e, 0x0f, 0x5e, 0x0d, 0x55,
			      0x2f, 0x1e, 0x85, 0xad},
	},
	[IPL_6106] = {
		.name = "6106 or 7106",
		.epOffset = -0x200000,
		.seed = 0x85 * 0x003d6e72u + 0x40,
		.txt = ipl_6106,
		.signature = {0x63, 0x46, 0x8e, 0x3a, 0xb5, 0x54, 0x25, 0x3d,
			      0xa3, 0x4d, 0xe3, 0x59, 0x5c, 0xd0, 0x9b, 0x0e,
			      0xce, 0xa1, 0xce, 0x00},
	},
	[IPL_7102] = {
		.name = "7102",
		.epOffset = 0,
		.seed = 0x3f * 0x5d588b65u + 1,
		.txt = ipl_7102,
		.signature = {0x79, 0xfe, 0x35, 0x1c, 0x50, 0xcd, 0xf7, 0x7c,
			      0x74, 0xe5, 0x03, 0xd7, 0x51, 0xa7, 0x15, 0x60,
			      0x5f, 0x87, 0xb8, 0x09},
	},
	[IPL_8303] = {
		.name = "8303",
		.epOffset = 0,
		.txt = ipl_8303,
		.signature = {0x5e, 0xa7, 0xfc, 0x32, 0x74, 0xbe, 0x01, 0x12,
			      0x1c, 0xfb, 0x20, 0xad, 0x6c, 0x9e, 0x60, 0x85,
			      0xd6, 0x32, 0x79, 0xbe},
	},
	[IPL_HW1] = {
		.name = "HW1",
		.epOffset = 0,
		.signature = {0xd6, 0x8c, 0x83, 0xda, 0xb8, 0xc2, 0xc2, 0xb0,
			      0x8e, 0x36, 0x70, 0xbd, 0xf2, 0x8d, 0x93, 0xb8,
			      0x94, 0x61, 0x20, 0xc6},
	},
	[IPL_IQUE] = {
		.name = "iQue",
		.epOffset = 0,
		.signature = {0x17, 0x6d, 0x20, 0xcd, 0x8b, 0x0f, 0x7e, 0x24,
			      0x5b, 0x3d, 0x4e, 0x02, 0x7e, 0x78, 0x95, 0x52,
			      0x01, 0xe7, 0xee, 0x5c},

	},
	{
		.end = true,
	},
};

char *gradeErrors[] = {
	"ok",
	"couldn't stat file",
	"couldn't open file",
	"file too small",
};

struct perm_iterator {
	int *order;
	signed char *direction;
	int num_elements;
	int swapped_idx;
};

int perm_iterator_init(struct perm_iterator *p, int n)
{
	if (n < 1)
		return -1;

	p->num_elements = n;
	p->order = calloc(n, sizeof(int));
	p->direction = calloc(n, sizeof(char));

	if (!p->order || !p->direction)
		return -1;

	for (int i = 0; i < n; i++) {
		p->order[i] = i;
	}
	for (int i = 1; i < n; i++) {
		p->direction[i] = (signed char)(-1);
	}
	p->direction[0] = 0;
	return 0;
}

void perm_iterator_destroy(struct perm_iterator *p)
{
	free(p->order);
	free(p->direction);
}

int perm_iterator_has_next(struct perm_iterator *p)
{
	int i;
	for (i = 0; i < p->num_elements; ++i)
		if (p->direction[i] != 0)
			return 1;
	return 0;
}

/*
 * Johnson-Trotter algorithm with Even's speedup.
 * Transcribed from prose on Wikipedia:
 * https://en.wikipedia.org/w/index.php?oldid=795078780#Even.27s_speedup
 */
int perm_iterator_iterate(struct perm_iterator *p)
{
	if (!perm_iterator_has_next(p))
		return -1;
	int n = p->num_elements;
	int *a = p->order;
	signed char *b = p->direction;

	int largest = 0;
	int lIdx = 0;
	int i, j, t;
	for (i = 0; i < n; i++) {
		if (a[i] > largest && b[i] != 0) {
			largest = a[i];
			lIdx = i;
		}
	}
	j = lIdx + b[lIdx];
	t = a[lIdx];
	a[lIdx] = a[j];
	a[j] = t;
	t = b[lIdx];
	b[lIdx] = b[j];
	b[j] = t;
	if ((j == (n - 1)) || (j == 0) || (a[j + b[j]] > a[j])) {
		b[j] = 0;
	}
	for (i = 0; i < n; i++) {
		if (a[i] > a[j]) {
			b[i] = (i < j) ? 1 : -1;
		}
	}
	t = lIdx > j ? j : lIdx;
	return p->swapped_idx = t;
}

int identify_ipl3(uint8_t * ipl3)
{
	uint8_t sha1sum[SHA1_DIGEST_SIZE];
	SHA1_CTX sha1ctx;
	SHA1_Init(&sha1ctx);
	SHA1_Update(&sha1ctx, ipl3, 0xFC0);
	SHA1_Final(&sha1ctx, sha1sum);
	int cicIndex = 0;
	while (!cics[cicIndex].end) {
		if (!memcmp(cics[cicIndex].signature, sha1sum,
			SHA1_DIGEST_SIZE)) {
			return cicIndex;
			break;
		}
		cicIndex++;
	}
	return IPL_NONE;
}

void swap4(uint8_t * buf, size_t len, int order)
{
	size_t i;
	int j;
	uint8_t tmp[4];
	int lut[4];
	lut[0] = order / 1000 % 10 - 1;
	lut[1] = order / 100  % 10 - 1;
	lut[2] = order / 10   % 10 - 1;
	lut[3] = order / 1    % 10 - 1;
	for (i = 0; i < len; i += 4) {
		for (j = 0; j < 4; j++)
			tmp[j] = buf[i + lut[j]];
		for (j = 0; j < 4; j++)
			buf[i + j] = tmp[j];
	}
}

void vis(struct romGrade *rg)
{
#ifndef _WIN32
	char **g = (char *[]){
		[GRADE_OK]		= "[\e[32mGOOD\e[0m]",
		[GRADE_WARN]		= "[\e[33mWARN\e[0m]",
		[GRADE_ERROR]		= "[\e[31mFAIL\e[0m]",
		[GRADE_NOT_GRADED]	= "[\e[35m NA \e[0m]",
		[GRADE_FIXED]		= "[\e[32mFIXD\e[0m]",
	};
#else
	char **g = (char *[]){
		[GRADE_OK]		= "[GOOD]",
		[GRADE_WARN]		= "[WARN]",
		[GRADE_ERROR]		= "[FAIL]",
		[GRADE_NOT_GRADED]	= "[ NA ]",
		[GRADE_FIXED]		= "[FIXD]",
	};
#endif
	printf("Game name:\t\t%s\n",
			strlen(rg->name)?rg->name:"(empty)");
	printf("Product code:\t\t%s\n",
			strlen(rg->productCode)?rg->productCode:"(empty)");
	printf("Entry point:\t\t%s %08x\n", g[rg->entrypointGrade],
			rg->entrypoint);
	printf("Byte order grade:\t%s %d\n", g[rg->byteOrderGrade],
			rg->byteOrder);
	printf("PI timings grade:\t%s %08x\n", g[rg->piTimingsGrade],
			rg->piTimings);
	printf("File size grade:\t%s 0x%x bytes\n", g[rg->fileSizeGrade],
			rg->fileSize);
	printf("CRC grade:\t\t%s %08x %08x\n", g[rg->crcGrade], rg->crc1_inrom,
			rg->crc2_inrom);
	printf("IPL3 grade:\t\t%s %s\n", g[rg->ipl3Grade],
		rg->ipl3Grade != GRADE_ERROR ? cics[rg->ipl3].name : "");
	if (rg->weirdIqueHeader) {
		printf("WARNING: weird 32-byte iQue header present.\n");
	}
}

void grade_size(struct romGrade *rg, uint8_t * rom, size_t len)
{
	(void)rom;

	rg->fileSize = len;
	if (rg->fileSize < 0x101000) {
		rg->fileSizeGrade = GRADE_ERROR;
		return;
	}
	switch (rg->fileSize) {
	case 4 * 1024 * 1024:
	case 8 * 1024 * 1024:
	case 12 * 1024 * 1024:
	case 16 * 1024 * 1024:
	case 20 * 1024 * 1024:	// Disney's Donald Duck - Going Quackers
	case 24 * 1024 * 1024:	// Mario Golf
	case 28 * 1024 * 1024:	// Hydro Thunder, WWF WrestleMania 2000
	case 32 * 1024 * 1024:
	case 40 * 1024 * 1024:	// Ogre Battle, Paper Mario
	case 64 * 1024 * 1024:	// Conker's BFD, Pokemon Stadium 2, RE2
		// Normal rom size.
		rg->fileSizeGrade = GRADE_OK;
		break;
	default:
		rg->fileSizeGrade = GRADE_WARN;
		break;
	}
}

void grade_byte_order(struct romGrade *rg, uint8_t * rom, size_t len)
{
	// Swap IPL3 around until it matches.
	rg->ipl3 = IPL_NONE;
	rg->ipl3Grade = GRADE_ERROR;
	if (len < 0x1000) return;

	struct perm_iterator p;
	if (perm_iterator_init(&p, 4) != 0) {
		fprintf(stderr, "perm_iterator_init\n");
		exit(1);
	}
	do {
		int i;
		uint8_t a[0x1000], b[4];
		for (i = 0x40; i < 0x1000; i += 4) {
			b[0] = rom[i + p.order[0]];
			b[1] = rom[i + p.order[1]];
			b[2] = rom[i + p.order[2]];
			b[3] = rom[i + p.order[3]];
			a[i + 0] = b[0];
			a[i + 1] = b[1];
			a[i + 2] = b[2];
			a[i + 3] = b[3];
		}
		rg->ipl3 = identify_ipl3(a + 0x40);
		if (rg->ipl3 != IPL_NONE)
			break;
	} while (perm_iterator_iterate(&p) >= 0);

	if (rg->ipl3 != IPL_NONE) {
		rg->ipl3Grade = GRADE_OK;
		rg->byteOrder = p.order[0] * 1000
			      + p.order[1] * 100
			      + p.order[2] * 10
			      + p.order[3] * 1;
		rg->byteOrder += 1111;
	}

	switch (rg->byteOrder) {
	case 1234:
		rg->byteOrderGrade = GRADE_OK;
		break;
	default:
		if (rg->fix)
			rg->byteOrderGrade = GRADE_FIXED;
		else
			rg->byteOrderGrade = GRADE_ERROR;
		break;
	}
	perm_iterator_destroy(&p);
}

void grade_pi_timings(struct romGrade *rg, uint8_t *rom, size_t len)
{
	// Check PI timings.
	
	if (len < 64) return;

	rg->piTimings = ntohl(((uint32_t *) rom)[0]);
	switch (rg->piTimings) {
	case 0x80371240:
		rg->piTimingsGrade = GRADE_OK;
		break;
	case 0x80270740:
		if (rg->ipl3 == IPL_8303) {
			rg->piTimingsGrade = GRADE_OK;
		} else {
			rg->piTimingsGrade = GRADE_WARN;
		}
		break;
	default:
		rg->piTimingsGrade = GRADE_WARN;
		break;
	}
}

void grade_crcs(struct romGrade *rg, uint8_t *rom, size_t len)
{
	uint32_t *rom32 = (uint32_t *)rom;

	if (len < 24) return;
	rg->crc1_inrom = ntohl(rom32[4]);
	rg->crc2_inrom = ntohl(rom32[5]);

	if (rg->ipl3Grade == GRADE_ERROR)
		return;
	
	if (len < 0x101000) return;

	uint32_t v1, t0, v0, a3, t2, t3, s0, a2, t4, t7, t5, t8, t6, a0, a1,
		t9, ra, t1;
	a0 = /* start of 1mb seg */ 0x1000;
	ra = 0x100000;
	v1 = 0;
	t0 = 0;
	t1 = a0;
	t5 = 0x20;
	v0 = cics[rg->ipl3].seed;
	a3 = v0;
	t2 = v0;
	t3 = v0;
	s0 = v0;
	a2 = v0;
	t4 = v0;

	// 80000130
	do {
		v0 = ntohl(rom32[t1 / 4]);
		v1 = a3 + v0;
		a1 = v1;
		if (v1 < a3) {
			t2 = t2 + 1;
		}

		// 80000148
		v1 = v0 & 0x1f;
		t7 = t5 - v1;
		if (t7 == 32)
			t8 = 0;
		else
			t8 = v0 >> t7;
		t6 = v0 << v1;
		a0 = t6 | t8;
		a3 = a1;
		t3 ^= v0;
		s0 += a0;
		if (a2 < v0) {
			t9 = a3 ^ v0;
			a2 = t9 ^ a2;
		} else {
			a2 ^= a0;
		}

		// 80000180
		if (rg->ipl3 == IPL_6105) {
			uint32_t addr = 0x750 + (t0 & 0xff);
			t4 += v0 ^ ntohl(rom32[addr / 4]);
		} else {
			t4 += v0 ^ s0;
		}

		t0 += 4;
		t1 += 4;
	} while (t0 != ra);

	switch (rg->ipl3) {
	case IPL_6101:
	case IPL_6102:
	case IPL_7102:
	case IPL_6105:
		rg->crc1_calculated = (a3 ^ t2) ^ t3;
		rg->crc2_calculated = (s0 ^ a2) ^ t4;
		break;
	case IPL_6103:
		rg->crc1_calculated = (a3 ^ t2) + t3;
		rg->crc2_calculated = (s0 ^ a2) + t4;
		break;
	case IPL_6106:
		rg->crc1_calculated = (a3 * t2) + t3;
		rg->crc2_calculated = (s0 * a2) + t4;
		break;
	case IPL_8303:
	default:
		rg->crc1_calculated = 0;
		rg->crc2_calculated = 0;
		break;
	}

	rom32[4] = htonl(rg->crc1_calculated);
	rom32[5] = htonl(rg->crc2_calculated);

	if ((rg->crc1_inrom == rg->crc1_calculated)
	 && (rg->crc2_inrom == rg->crc2_calculated)) {
		rg->crcGrade = GRADE_OK;
	} else {
		if (rg->fix) {
			rg->crc1_inrom = rg->crc1_calculated;
			rg->crc2_inrom = rg->crc2_calculated;
			rg->crcGrade = GRADE_FIXED;
		} else {
			rg->crcGrade = GRADE_ERROR;
		}
	}
}

void grade_name(struct romGrade *rg, uint8_t * rom, size_t len)
{
	if (len < 64) return;

	memcpy(&rg->name, rom + 32, 20);
	rg->name[20] = '\0';
	for (int i=0; i<20; i++) {
		char *c = &rg->name[i];
		if ((*c < ' ') || (*c > '~')) {
			*c = '\0';
			break;
		}
	}
}

void grade(struct romGrade *rg, uint8_t * rom, size_t len, enum ipl_e force_ipl, uint8_t force_region)
{
	if (len < 64) return;

	// Detect presence of optional iQue header and skip it if present.
	if (0x80371240 == ntohl(((uint32_t *) rom)[8])) {
		rom += 32;
		len -= 32;
		rg->weirdIqueHeader = true;
	}
	
	// Grade rom size.
	grade_size(rg, rom, len);

	// Grade byte order and IPL3.
	if (len < 0x1000) return;
	grade_byte_order(rg, rom, len);	// also grades ipl3

	// Byteswap the ROM if necessary.
	if (rg->ipl3 != IPL_NONE && rg->byteOrder != 1234) {
		swap4(rom, rg->fileSize, rg->byteOrder);
	}

	// Grade entrypoint.
	rg->entrypoint = ntohl(((uint32_t *) rom)[2]);
	if (rg->ipl3 > 0)
		rg->entrypoint += cics[rg->ipl3].epOffset;

	if (	((rg->entrypoint & 0xf) == 0) &&
		(rg->entrypoint >= 0x80000000) &&
		(rg->entrypoint <= 0x80700000) ) {
			rg->entrypointGrade = GRADE_OK;
	} else {
		rg->entrypointGrade = GRADE_ERROR;
	}


	// Force IPL3?
	if ((force_ipl != IPL_NONE) && (force_ipl != rg->ipl3)) {
		uint32_t ep = ntohl(((uint32_t *)rom)[2]);

		if (cics[force_ipl].txt == NULL)
			errx(1, "can't force IPL '%s'", cics[force_ipl].name);
		memcpy(rom + 0x40, cics[force_ipl].txt, 0xfc0);
		
		if (rg->ipl3 != IPL_NONE) {
			ep += cics[rg->ipl3].epOffset;
		}

		ep -= cics[force_ipl].epOffset;
		((uint32_t *)rom)[2] = htonl(ep);
		rg->ipl3 = force_ipl;
		rg->ipl3Grade = GRADE_FIXED;
	}


	// Force region?
	if (force_region != '\0') {
		((uint8_t *)rom)[0x3e] = force_region;
	}

	// Copy product code into rg structure.
	memcpy(rg->productCode, rom + 0x3b, 4);
	switch (rom[0x3f]) {
	case 0 ... 9:
		rg->productCode[4] = rom[0x3f] + '0';
		rg->productCode[5] = '\0';
		break;
	default:
		rg->productCode[4] = '\\';
		rg->productCode[5] = 'x';
		rg->productCode[6] = "0123456789abcdef"[rom[0x3f] >> 4];
		rg->productCode[7] = "0123456789abcdef"[rom[0x3f] & 0x0f];
		rg->productCode[8] = '\0';
		break;
	}

	// More gradings.
	grade_pi_timings(rg, rom, len);
	grade_name(rg, rom, len);
	grade_crcs(rg, rom, len);
}
