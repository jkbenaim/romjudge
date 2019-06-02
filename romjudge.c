/* jrra 2019 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "grader.h"
#include "mapfile.h"

#define die(...) {				\
	fprintf(stderr,"%s: ", argv[0]);	\
	fprintf(stderr, __VA_ARGS__);		\
	fprintf(stderr,"\n");			\
	exit(1);				\
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		die("need a filename");
	}

	char *filename = argv[1];

	struct MappedFile_s m = mapfile(filename, false);

	if (m.data == NULL) {
		fprintf(stderr, "couldn't open file: %s\n", filename);
		return EXIT_FAILURE;
	}

	struct romGrade rg = {0};

	grade(&rg, (uint8_t *)m.data, (size_t)m.size);
	vis(&rg);
	unmapfile(m);
	return EXIT_SUCCESS;
}
