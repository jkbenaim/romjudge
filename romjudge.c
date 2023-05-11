/* jrra 2023 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <stdnoreturn.h>
#include <iso646.h>
#include <err.h>
#include "grader.h"
#include "mapfile.h"

extern char *__progname;

noreturn static void usage(void)
{
	(void)fprintf(stderr,
		"usage: %s [-c ipl] [-r region] -f file\n"
		"  -c will force an IPL and rewrite checksums.\n"
		"  -r will overwrite the region letter.\n"
		"\n"
		"Report bugs to:\n"
		"https://github.com/jkbenaim/romjudge\n",
		__progname
	);
	exit(EXIT_FAILURE);
}

enum ipl_e IPLdToEnum(unsigned u)
{
	switch (u) {
	case 6101:
		return IPL_6101;
		break;
	case 6102:
	case 7101:
		return IPL_6102;
		break;
	case 6103:
	case 7103:
		return IPL_6103;
		break;
	case 6105:
	case 7105:
		return IPL_6105;
		break;
	case 6106:
	case 7106:
		return IPL_6106;
		break;
	case 8303:
		return IPL_8303;
		break;
	default:
		return IPL_NONE;
		break;
	};
}

int main(int argc, char *argv[])
{
	int rc;
	char *filename = NULL;
	struct MappedFile_s m = {0};
	struct romGrade rg = {0};
	bool fix = false;
	enum ipl_e force_ipl = IPL_NONE;
	unsigned char force_region = '\0';

	while ((rc = getopt(argc, argv, "f:c:r:")) != -1)
		switch (rc) {
		case 'f':
			if (filename)
				usage();
			filename = optarg;
			break;
		case 'c':
			if (force_ipl != IPL_NONE)
				usage();
			force_ipl = IPLdToEnum(strtoul(optarg, NULL, 10));
			break;
		case 'r':
			if (force_region != '\0')
				usage();
			force_region = optarg[0];
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (not filename)
		usage();
	if (*argv != NULL)
		usage();

	fix = (force_ipl != IPL_NONE) or force_region;

	m = MappedFile_Open(filename, fix);
	if (!m.data) err(1, "couldn't open '%s' for reading", filename);

	rg.fix = fix;
	grade(&rg, (uint8_t *)m.data, (size_t)m.size, force_ipl, force_region);
	vis(&rg);
	MappedFile_Close(m);
	m.data = NULL;
	return EXIT_SUCCESS;
}
