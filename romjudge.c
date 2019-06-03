/* jrra 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "grader.h"
#include "mapfile.h"

void print_usage()
{
	printf(
		"Usage: romjudge [-f] file\n"
		"  -f will fix a broken ROM.\n"
		"\n"
		"Report bugs to:\n"
		"https://github.com/jkbenaim/romjudge\n"
	);
}

int main(int argc, char *argv[])
{
	struct romGrade rg = {0};
	char *filename;

	if (argc < 2) {
		fprintf(stderr, "error: need a filename\n");
		print_usage();
		return EXIT_FAILURE;
	}


	if (!strcmp(argv[1],"-f")) {
		if (argc < 3) {
			fprintf(stderr, "error: need a filename\n");
			print_usage();
			return EXIT_FAILURE;
		}
		rg.fix = true;
		filename = argv[2];
	} else {
		rg.fix = false;
		filename = argv[1];
	}

	struct MappedFile_s m = mapfile(filename, rg.fix);

	if (m.data == NULL) {
		fprintf(stderr, "error: couldn't open file: %s\n", filename);
		return EXIT_FAILURE;
	}


	grade(&rg, (uint8_t *)m.data, (size_t)m.size);
	vis(&rg);
	unmapfile(m);
	return EXIT_SUCCESS;
}
