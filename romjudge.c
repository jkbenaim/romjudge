// jrra 2017

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include "grader.h"

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

	struct stat sb;
	// Check that the file actually exists.
	if (stat(filename, &sb) == -1)
		die("couldn't stat file '%s'", filename);

	// Mmap the file.
	int f = open(filename, O_RDONLY);
	if (!f)
		return 2;	// "Couldn't open file."
	uint8_t *rom = (uint8_t *) mmap(NULL, sb.st_size,
					PROT_READ | PROT_WRITE,
					MAP_PRIVATE, f, 0);

	struct romGrade *rg = calloc(1, sizeof(struct romGrade));
	grade(rg, rom, sb.st_size);
	vis(rg);
	free(rg);
	munmap(rom, sb.st_size);
	close(f);
	return 0;
}
