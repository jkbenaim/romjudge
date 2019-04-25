// jrra 2017

#ifndef _GRADER_H_
#define _GRADER_H_

#include <inttypes.h>
#include <stdbool.h>
#include "sha1.h"

extern char *gradeErrors[];

enum GRADE {
	GRADE_NOT_GRADED,
	GRADE_OK,
	GRADE_WARN,
	GRADE_ERROR,
};

struct romGrade {
	char productCode[6];
	int byteOrder;
	int byteOrderGrade;
	uint32_t piTimings;
	int piTimingsGrade;
	uint32_t fileSize;
	int fileSizeGrade;
	uint32_t crc1_inrom;
	uint32_t crc2_inrom;
	uint32_t crc1_calculated;
	uint32_t crc2_calculated;
	int crcGrade;
	int ipl3;		// CIC number, or 0 for unknown, or 2 for HW1 ipl3
	int ipl3Grade;
	uint32_t entrypoint;
};

extern void grade(struct romGrade *rg, uint8_t * rom, size_t len);
extern void vis(struct romGrade *rg);

#endif				// _GRADER_H_
