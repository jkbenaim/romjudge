// jrra 2017

#ifndef _GRADER_H_
#define _GRADER_H_

#include <inttypes.h>
#include <stdbool.h>
#include "sha1.h"

struct cicSignature {
	int type;
	char *name;
	uint8_t signature[SHA1_DIGEST_SIZE];
};

extern struct cicSignature cics[];
extern char *gradeErrors[];

enum GRADE {
	GRADE_NOT_GRADED,
	GRADE_OK,
	GRADE_WARN,
	GRADE_ERROR,
};

struct romGrade {
	int byteOrder;
	int byteOrderGrade;
	uint32_t piTimings;
	int piTimingsGrade;
	uint32_t fileSize;
	int fileSizeGrade;
	uint32_t crc1;
	uint32_t crc2;
	int crcGrade;
	int ipl3;		// CIC number, or 0 for unknown, or 2 for HW1 ipl3
	int ipl3Grade;
};

extern void grade(struct romGrade *rg, uint8_t * rom, size_t len,
		 bool swapWholeRom);
extern void vis(struct romGrade *rg);

#endif				// _GRADER_H_
