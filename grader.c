// jrra 2017

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "endian.h"
#include "sha1.h"
#include "grader.h"

struct cicSignature cics[] = {
	{
		.type = 1,
		.name = "unrecognized",
		.signature = {0},
	},
	{
		.type = 6101,
		.name = "6101",
		.signature = {0xea, 0xad, 0xcb, 0x8c, 0xca, 0x9c, 0x6b, 0xa1,
			      0x44, 0x5f, 0x98, 0xf1, 0x72, 0x7b, 0xf4, 0xad,
			      0xba, 0xbb, 0x88, 0xb2},
	},
	{
		.type = 6102,		// and 7101
		.name = "6102 or 7101",
		.signature = {0xb2, 0xaf, 0xae, 0x24, 0x6e, 0x1d, 0xab, 0x74,
			      0x6b, 0xfb, 0x28, 0xcb, 0x34, 0x6e, 0x29, 0x11,
			      0x96, 0x5e, 0xef, 0xa1},
	},
	{
		.type = 6103,		// and 7103
		.name = "6013 or 7103",
		.signature = {0x3f, 0x73, 0x47, 0xaa, 0x04, 0x26, 0xee, 0x97,
			      0xd9, 0x67, 0x21, 0xb0, 0x9b, 0x91, 0x8d, 0x4c,
			      0x9c, 0xdd, 0xb6, 0x9b},
	},
	{
		.type = 6105,		// and 7105
		.name = "6105 or 7105",
		.signature = {0x41, 0x59, 0x26, 0x90, 0x55, 0xe8, 0xa5, 0xbe,
			      0x2e, 0x5c, 0x8e, 0x3e, 0x0f, 0x5e, 0x0d, 0x55,
			      0x2f, 0x1e, 0x85, 0xad},
	},
	{
		.type = 6106,		// and 7106
		.name = "6106 or 7106",
		.signature = {0x63, 0x46, 0x8e, 0x3a, 0xb5, 0x54, 0x25, 0x3d,
			      0xa3, 0x4d, 0xe3, 0x59, 0x5c, 0xd0, 0x9b, 0x0e,
			      0xce, 0xa1, 0xce, 0x00},
	},
	{
		.type = 7102,		// TODO: double-check this
		.name = "7102",
		.signature = {0x79, 0xfe, 0x35, 0x1c, 0x50, 0xcd, 0xf7, 0x7c,
			      0x74, 0xe5, 0x03, 0xd7, 0x51, 0xa7, 0x15, 0x60,
			      0x5f, 0x87, 0xb8, 0x09},
	},
	{
		.type = 8303,
		.name = "8303",
		.signature = {0x5e, 0xa7, 0xfc, 0x32, 0x74, 0xbe, 0x01, 0x12,
			      0x1c, 0xfb, 0x20, 0xad, 0x6c, 0x9e, 0x60, 0x85,
			      0xd6, 0x32, 0x79, 0xbe},
	},
	{
		.type = 2,		/* SGI/hw1 bootcode? */
		.name = "HW1",
		.signature = {0xd6, 0x8c, 0x83, 0xda, 0xb8, 0xc2, 0xc2, 0xb0,
			      0x8e, 0x36, 0x70, 0xbd, 0xf2, 0x8d, 0x93, 0xb8,
			      0x94, 0x61, 0x20, 0xc6},
	},
	{
		/* The list is terminated by a fake CIC with type 0. */
		.type = 0,
		.signature = {0},
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
	while (cics[cicIndex].type != 0) {
		if (!memcmp(cics[cicIndex].signature, sha1sum, SHA1_DIGEST_SIZE)){
			return cicIndex;
			break;
		}
		cicIndex++;
	}
	return -1;
}

void swap4(uint8_t * buf, size_t len, int order)
{
	size_t i;
	int j;
	uint8_t tmp[4];
	int lut[4];
	lut[0] = order/1000 % 10 - 1;
	lut[1] = order/100  % 10 - 1;
	lut[2] = order/10   % 10 - 1;
	lut[3] = order/1    % 10 - 1;
	for (i = 0; i < len; i += 4) {
		for (j = 0; j < 4; j++)
			tmp[j] = buf[i + lut[j]];
		for (j = 0; j < 4; j++)
			buf[i + j] = tmp[j];
	}
}

void vis(struct romGrade *rg)
{
	char **g = (char *[]){
		[GRADE_OK]		= "[\e[32mGOOD\e[0m]",
		[GRADE_WARN]		= "[\e[33mWARN\e[0m]",
		[GRADE_ERROR]		= "[\e[31mFAIL\e[0m]",
		[GRADE_NOT_GRADED]	= "[\e[35m NA \e[0m]",
	};
	printf("Byte order grade:\t%s %d\n", g[rg->byteOrderGrade],
			rg->byteOrder);
	printf("PI timings grade:\t%s %08x\n", g[rg->piTimingsGrade],
			rg->piTimings);
	printf("File size grade:\t%s 0x%x bytes\n", g[rg->fileSizeGrade],
			rg->fileSize);
	printf("CRC grade:\t\t%s %08x %08x\n", g[rg->crcGrade], rg->crc1,
			rg->crc2);
	printf("IPL3 grade:\t\t%s %s\n", g[rg->ipl3Grade],
		rg->ipl3Grade != GRADE_ERROR ? cics[rg->ipl3].name : "");
}

void grade_size(struct romGrade *rg, uint8_t * rom, size_t len)
{
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
	rg->ipl3 = 0;
	rg->ipl3Grade = GRADE_ERROR;
	struct perm_iterator p;
	perm_iterator_init(&p, 4);
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
		if (rg->ipl3 != -1)
			break;
	} while (perm_iterator_iterate(&p) >= 0);

	if (rg->ipl3 != -1) {
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
		rg->byteOrderGrade = GRADE_WARN;
		break;
	}
	perm_iterator_destroy(&p);
}

void grade_pi_timings(struct romGrade *rg, uint8_t *rom, size_t len)
{
	// Check PI timings.
	rg->piTimings = be32toh(((uint32_t *) rom)[0]);
	switch (rg->piTimings) {
	case 0x80371240:
		rg->piTimingsGrade = GRADE_OK;
		break;
	case 0x80270740:
		if (cics[rg->ipl3].type == 8303) {
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
	// TODO: this doesn't check that the CRCs are correct...
	rg->crc1 = be32toh(((uint32_t *) rom)[4]);
	rg->crc2 = be32toh(((uint32_t *) rom)[5]);
	if (rg->ipl3Grade == GRADE_OK) {
	switch (cics[rg->ipl3].type) {
	case 8303:
		if (rg->crc1 == 0 && rg->crc2 == 0)
			rg->crcGrade = GRADE_OK;
		break;
	default:
		break;
	}
	}
}

void grade(struct romGrade *rg, uint8_t * rom, size_t len, bool swapWholeRom)
{
	grade_size(rg, rom, len);
	if (rg->fileSizeGrade == GRADE_ERROR)
		return;

	grade_byte_order(rg, rom, len);	// also grades ipl3
	if (rg->byteOrderGrade == GRADE_ERROR)
		return;

	// Byteswap the ROM if necessary.
	if (rg->ipl3 != -1 && rg->byteOrder != 1234) {
		size_t bytes_to_swap;
		if (swapWholeRom)
			bytes_to_swap = rg->fileSize;
		else
			bytes_to_swap = 0x101000;
		swap4(rom, bytes_to_swap, rg->byteOrder);
	}

	grade_pi_timings(rg, rom, len);
	grade_crcs(rg, rom, len);
}
