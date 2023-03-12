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
	GRADE_FIXED
};

enum ipl_e {
	IPL_NONE = 0,
	IPL_6101,
	IPL_6102,
	IPL_6103,
	IPL_6105,
	IPL_6106,
	IPL_7102,
	IPL_8303,
	IPL_HW1,
	IPL_IQUE,
};

struct romGrade {
	bool fix;
	char productCode[9];	// "CZLE\x10"
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
	int entrypointGrade;
	char name[21];
};

extern void grade(struct romGrade *rg, uint8_t * rom, size_t len, enum ipl_e force_ipl, uint8_t force_region);
extern void vis(struct romGrade *rg);

#endif				// _GRADER_H_
